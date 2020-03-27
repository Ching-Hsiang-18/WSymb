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
    k1 < k2
  else
    less_than_loop l1 l2

let rec less_than_awcet w1 w2 =
  match (w1,w2) with
    | ([], last1), ([], last2) -> last1 < last2
    | _,_ ->
       let hd1, hd2= (hd w1), (hd w2) in
       if (hd w1) <> (hd w2) then
         hd1 < hd2
       else
         less_than_awcet (tl w1) (tl w2)
  
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
       k1 < k2
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

let term_of_prod f =
  match f with
  | FProduct (_, f') -> Some f'
  | FConst _ -> None
  | _ -> Some f

let const_of_prod f =
  match f with
  | FProduct (k, _) -> k
  | FConst _ -> internal_error "const_of_prod" "f should not be const"
  | _ -> 1

let rec simplify_product (k,f) =
  if k = 0 then
    FConst bot_wcet
  else match f with
       | FConst c ->
          FConst (prod k c)
       | FProduct (k',f') ->
          simplify_product (k*k',f')
       | _ -> FProduct (k,f)

let term_of_sum f =
  match f with
  | FPlus ((FConst _)::rest) -> Some (FPlus rest)
  | FConst _ -> None
  | _ -> Some f

let const_of_sum f =
  match f with
  | FPlus [FConst c; _] -> FConst c
  | FConst _ -> internal_error "const_of_sum" "f should not be const"
  | _ -> FConst bot_wcet
  
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
  | [f1; f2] when (term_of_prod f1=term_of_prod f2 && term_of_prod f1 <> None) ->
     let t = match term_of_prod f1 with
       | Some t' -> t'
       | None -> internal_error "simplify_sum_rec" "cannot be none"
     in
     [FProduct (((const_of_prod f1)+(const_of_prod f2)), t)]
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
          
and simplify_sum fl =
  match fl with
  | [] -> internal_error "simplify_sum" "empty list"
  | [f1] -> f1 (* Might happen due to neutral element bot_f *)
  | _ ->
     let fl' = simplify_sum_rec fl in
     match fl' with
     | [] -> internal_error  "simplify_sum" "empty list"
     | [f1] -> f1
     | _ -> FPlus fl'

(* Assumes that List.length f >= 2 *)          
and simplify_union_rec fl =
  match fl with
  | [] | [_] -> internal_error "simplify_union_rec" "wrong list size"
  | [FUnion fl1; FUnion fl2] ->
     merge_unions fl1 fl2
  | [FUnion fl; other] ->
     merge_unions fl [other]
  | [other; FUnion fl] ->
     merge_unions [other] fl
  | [FConst w1; FConst w2] ->
     if w1 = bot_wcet then [FConst w2]
     else if w2 = bot_wcet then [FConst w1]
     else [FConst (Abstract_wcet.union w1 w2)]
  | [f1; f2] when (term_of_sum f1=term_of_sum f2 && term_of_sum f1 <> None) ->
     let t = match term_of_sum f1 with
       | Some t' -> t'
       | None -> internal_error "simplify_union_rec" "cannot be none"
     in
     let u = FUnion [simplify_union [const_of_sum f1; const_of_sum f2]] in
     [simplify_sum [u; t]]
  | [f1; f2] ->
     if less_than_f f2 f1 then
       [f2; f1]
     else [f1; f2]
  | (FUnion fl)::tl ->
     merge_unions fl (simplify_union_rec tl)
  | hd::tl ->
     merge_unions [hd] (simplify_union_rec tl)

and merge_unions fl1 fl2 =
  match fl1,fl2 with
  | [],fl | fl,[] -> fl
  | hd1::tl1, hd2::tl2 ->
     let l=simplify_union_rec ([hd1;hd2]) in
     match l with
     | [] -> internal_error "merge_unions" "empty list"
     | [hd] -> hd::(merge_unions tl1 tl2)
     | [hd1';hd2'] when hd1'=hd1 ->
        hd1::(merge_unions tl1 fl2)
     | [hd1';hd2'] when hd1'=hd2 ->
        hd2::(merge_unions fl1 tl2)       
     | _ -> internal_error "merge_unions" "wrong list size"
          
and simplify_union fl =
  match fl with
  | [] -> internal_error "simplify_union" "empty list"
  | [f1] -> f1 (* Might happen due to neutral element bot_f *)
  | _ ->
     let fl' = simplify_union_rec fl in
     match fl' with
     | [] -> internal_error  "simplify_union" "empty list"
     | [f1] -> f1
     | _ -> FUnion fl'

let simplify_annot (f,a) =
  match f with
  | FConst c ->
     FConst (Abstract_wcet.annot c a)
  | _ ->
     if f = bot_f then bot_f
     else
       FAnnot (f,a)
          
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
  | FUnion fl ->
     simplify_union fl
  | FProduct (k,f) ->
     simplify_product (k,f)
  | FAnnot (f, a) ->
     simplify_annot (f,a)
  | _ -> f'
           
