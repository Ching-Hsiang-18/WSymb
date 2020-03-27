open Utils
open Abstract_wcet
open Wcet_formula   

(* Based on the book "Computer Algebra and Symbolic Computation:
     Mathematical Methods", J.S. Cohen, 2002, Section 3.2 *)
   
(* Ordering functions to decide when to apply the commutativity rule *)      
          
let less_than_loop l1 l2 =
  match (l1, l2) with
  | LNamed n1, LNamed n2 -> n1 < n2
  | LParam p1, LParam p2 -> p1 < p2
  | _,LTop -> true
  | _,_ -> false

let less_than_param p1 p2 =
  p1 < p2
  
let less_than_sint i1 i2 =
  match (i1,i2) with
  | SInt k1, SInt k2 -> k1 < k2
  | SParam p1, SParam p2 -> less_than_param p1 p2
  | _,_ -> false

let less_than_annot (l1,k1) (l2,k2) =
  if (k1 <> k2) then
    less_than_sint k1 k2
  else
    less_than_loop l1 l2

let rec less_than_f f1 f2 =
  match (f1, f2) with
  | FConst (l1,w1), FConst (l2,w2) ->
     if (w1 <> w2) then
       less_than_awcet w1 w2
     else
       less_than_loop l1 l2
  | FParam p1, FParam p2 ->
     less_than_param p1 p2
  | (FPlus flist1, FPlus flist2)
  | (FUnion flist1, FUnion flist2) ->
     let rec aux fl1 fl2 =
       match fl1, fl2 with
         [],[] -> false
       | [],_ -> true
       | _,[] -> false
       | hd1::tl1, hd2::tl2 ->
          if (hd1 <> hd2) then
            less_than_f hd1 hd2
          else
            aux tl1 tl2
     in
     aux (List.rev flist1) (List.rev flist2)
  | (FProduct (k1, f1'), FProduct (k2, f2')) ->
     if (f1' <> f2') then
       (less_than_f f1' f2')
     else
       less_than_sint k1 k2
  | (FPower (f1sub1, f1sub2, l1, k1), FPower (f2sub1, f2sub2, l2, k2)) ->
     if (k1 <> k2) then
       less_than_sint k1 k2
     else
       if (l1 <> l2) then
         less_than_loop l1 l2
       else
         if (f1sub2 <> f2sub2) then
           less_than_f f1sub2 f2sub2
         else
           less_than_f f1sub1 f2sub1
  | FAnnot (f1, a1), FAnnot (f2, a2) ->
     if (a1 <> a2) then
       less_than_annot a1 a2
     else
       less_than_f f1 f2
  | FConst _, _ -> true
  | FProduct _, FPlus _ | FProduct _, FUnion _ | FProduct _, FPower _ | FProduct _, FAnnot _ | FProduct _, FParam _ -> true
  | FPlus _, FUnion _ | FPlus _, FPower _ | FPlus _, FAnnot _ | FPlus _, FParam _ -> true
  | FUnion _, FPower _ | FUnion _, FAnnot _ | FUnion _, FParam _ -> true
  | FPower _, FAnnot _ | FPower _, FParam _ -> true
  | FAnnot _, FParam _ -> true
  | _ -> not(less_than_f f2 f1)

(* Assumes that List.length f >= 2 *)       
let rec simplify_sum_rec fl =
  match fl with
  | [] | [_] -> internal_error "simplify_sum_rec" "wrong list size"
  | [FPlus fl1; FPlus fl2] ->
     merge_sums fl1 fl2
  | [FPlus fl; other] ->
     merge_sums fl [other]
  | [other; FPlus fl] ->
     merge_sums [other] fl
  | [FConst w1; FConst w2] ->
     if w1 = bot_wcet then [FConst w2]
     else if w2 = bot_wcet then [FConst w1]
     else [FConst (Abstract_wcet.sum w1 w2)]
  | [f1; f2] ->
     if less_than_f f2 f1 then
       [f2; f1]
     else [f1; f2]
  | (FPlus fl)::tl ->
     merge_sums fl (simplify_sum_rec tl)
  | hd::tl ->
     merge_sums [hd] (simplify_sum_rec tl)
    
and merge_sums fl1 fl2 =
  match fl1,fl2 with
  | [],fl | fl,[] -> fl
  | hd1::tl1, hd2::tl2 ->
     let l=simplify_sum_rec ([hd1;hd2]) in
     match l with
     | [] -> internal_error "merge_sums" "empty list"
     | [hd] -> hd::(merge_sums tl1 tl2)
     | [hd1';hd2'] when hd1'=hd1 ->
        hd1::(merge_sums tl1 fl2)
     | [hd1';hd2'] when hd1'=hd2 ->
        hd2::(merge_sums fl1 tl2)       
     | _ -> internal_error "merge_sums" "wrong list size"
          
(* Simplification functions *)      
let simplify_sum fl =
  match fl with
  | [] -> internal_error "simplify_sum" "empty list"
  | [f1] -> f1 (* Might happen due to neutral element bot_f *)
  | _ ->
     let fl' = simplify_sum_rec fl in
     match fl' with
     | [] -> internal_error  "simplify_sum" "empty list"
     | [f1] -> f1
     | _ -> FPlus fl'

let fmap fct formula =
  match formula with
  | FConst _ | FParam _ -> formula
  | FPlus fl ->
     FPlus (List.map fct fl)
  | FUnion fl ->
     FUnion (List.map fct fl)
  | FPower (f1, f2, l, it) ->
     FPower (fct f1, fct f2, l, it)
  | FAnnot (f, a) ->
     FAnnot (fct f, a)
  | FProduct (k, f) ->
     FProduct (k, fct f)
    
(* Main function *)
let rec simplify f =
  let f' = fmap simplify f in
  match f' with
  | FConst _ | FParam _ -> f'
  | FPlus fl ->
     simplify_sum fl
  | _ -> f'
           
(* let simplify f =
 *   (\* Brute force implementation *\)
 *   let simplified = ref true in
 *   let rec aux f =
 *     match f with
 *     (\* Associativity => requires nary plus/union *\)
 *     (\* Commutativity *\)
 *     | FPlus (f1, f2) when (less_than_f f1 f2) ->
 *        simplified := true;
 *        FPlus (aux f2, aux f1)
 *     | FUnion (f1, f2) when (less_than_f f1 f2) ->
 *        simplified := true;
 *        FUnion (aux f2, aux f1)
 *     (\* Distributivity *\)
 *     | FUnion (FPlus (FConst c1, f1), FPlus (FConst c2, f2)) when f1=f2 ->
 *        simplified := true;
 *        FPlus (FUnion (FConst c1, FConst c2), aux f1)
 *     (\* Neutral element *\)
 *     | FPlus (f1, f2) when f2=bot_f ->
 *        simplified := true;
 *        f1
 *     | FUnion (f1, f2) when f2=bot_f ->
 *        simplified := true;
 *        f1
 *     (\* Multiplication *\)
 *     | FProduct ((SInt 0), f) ->
 *        simplified := true;
 *        bot_f
 *     | FPlus ((FProduct ((SInt k), f1)), f2) when f1=f2 ->
 *        simplified := true;
 *        FProduct ((SInt (k+1)), aux f1)
 *     | FPlus ((FProduct ((SInt k), f1)), (FProduct ((SInt k'), f2))) when f1=f2 ->
 *        simplified := true;
 *        FProduct ((SInt (k+k')), aux f1)
 *     (\* Annotations *\)
 *     | FAnnot (f, _) when f=bot_f ->
 *        simplified := true;
 *        bot_f
 *     | FPlus (FAnnot (f1, a1), FAnnot (f2,a2)) when a1=a2 ->
 *        simplified := true;
 *        FAnnot (FPlus (aux f1, aux f2), a1)
 *     (\* Loops? *\)
 *     | _ -> f
 *   in
 *   let simpl_f = ref f in
 *   while !simplified do
 *     simplified := false;
 *     simpl_f := aux !simpl_f
 *   done;
 *   !simpl_f *)
