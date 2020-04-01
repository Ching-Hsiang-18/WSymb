(* ----------------------------------------------------------------------------
 * Copyright (C) 2020, Université de Lille, Lille, FRANCE
 *
 * This file is part of CFTExtractor.
 *
 * CFTExtractor is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation ; either version 2 of
 * the License, or (at your option) any later version.
 *
 * CFTExtractor is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program ; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *---------------------------------------------------------------------------- *)

open Utils
open Loops
open Abstract_wcet
open Wcet_formula   

(* Based on the book "Computer Algebra and Symbolic Computation:
     Mathematical Methods", J.S. Cohen, 2002, Section 3.2 *)
   
(* First, ordering functions to decide when to apply the commutativity
   rule. This effectively groups like-formulas, enabling factorization
   through inverse distributivity. *)

let less_than_loop l1 l2 =
  match (l1, l2) with
  | LNamed n1, LNamed n2 -> n1 < n2
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

(* Extract the term of a product, for inverse prod-sum distributivity
   simplification. *)       
let term_of_prod f =
  match f with
  | FProduct (_, f') -> Some f'
  | FConst _ -> None
  | _ -> Some f

(* Extract the const (factor) of a product, for inverse prod-sum distributivity. *)       
let const_of_prod f =
  match f with
  | FProduct (k, _) -> k
  | FConst _ -> internal_error "const_of_prod" "f should not be const"
  | _ -> 1
  
(* Assumes that [List.length fl] >= 2 *)       
let rec simplify_sum_rec loops fl =
  match fl with
  | [] | [_] -> internal_error "simplify_sum_rec" "wrong list size"
  | [FPlus fl1; FPlus fl2] ->
     merge_sums loops fl1 fl2
  | [FPlus fl; other] ->
     merge_sums loops fl [other]
  | [other; FPlus fl] ->
     merge_sums loops [other] fl
  | [FConst w1; FConst w2] ->
     if w1 = bot_wcet then [FConst w2]
     else if w2 = bot_wcet then [FConst w1]
     else [FConst (Abstract_wcet.sum loops w1 w2)]
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
     merge_sums loops fl (simplify_sum_rec loops tl)
  | hd::tl ->
     merge_sums loops [hd] (simplify_sum_rec loops tl)
    
and merge_sums loops fl1 fl2 =
  match fl1,fl2 with
  | [],fl | fl,[] -> fl
  | hd1::tl1, hd2::tl2 ->
     let l=simplify_sum_rec loops ([hd1;hd2]) in
     match l with
     | [] -> internal_error "merge_sums" "empty list"
     | [hd] -> hd::(merge_sums loops tl1 tl2)
     | [hd1';hd2'] when hd1'=hd1 ->
        hd1::(merge_sums loops tl1 fl2)
     | [hd1';hd2'] when hd1'=hd2 ->
        hd2::(merge_sums loops fl1 tl2)       
     | _ -> internal_error "merge_sums" "wrong list size"
          
and simplify_sum loops fl =
  match fl with
  | [] -> internal_error "simplify_sum" "empty list"
  | [f1] -> f1 (* Might happen due to neutral element bot_f *)
  | _ ->
     let fl' = simplify_sum_rec loops fl in
     match fl' with
     | [] -> internal_error  "simplify_sum" "empty list"
     | [f1] -> f1
     | _ -> FPlus fl'

(* Extract the term of a sum, for inverse union-sum distributivity
   simplification. *)
let term_of_sum f =
  match f with
  | FPlus ((FConst _)::rest) -> Some (FPlus rest)
  | FConst _ -> None
  | _ -> Some f

(* Extract the const of a sum, for inverse union-sum distributivity
   simplification. *)              
let const_of_sum f =
  match f with
  | FPlus [FConst c; _] -> FConst c
  | FConst _ -> internal_error "const_of_sum" "f should not be const"
  | _ -> FConst bot_wcet
          
(* Assumes that [List.length fl] >= 2 *)          
let rec simplify_union_rec loops fl =
  match fl with
  | [] | [_] -> internal_error "simplify_union_rec" "wrong list size"
  | [FUnion fl1; FUnion fl2] ->
     merge_unions loops fl1 fl2
  | [FUnion fl; other] ->
     merge_unions loops fl [other]
  | [other; FUnion fl] ->
     merge_unions loops [other] fl
  | [FConst w1; FConst w2] ->
     if w1 = bot_wcet then [FConst w2]
     else if w2 = bot_wcet then [FConst w1]
     else [FConst (Abstract_wcet.union loops w1 w2)]
  | [f1; f2] when (term_of_sum f1=term_of_sum f2 && term_of_sum f1 <> None) ->
     let t = match term_of_sum f1 with
       | Some t' -> t'
       | None -> internal_error "simplify_union_rec" "cannot be none"
     in
     let u = FUnion [simplify_union loops [const_of_sum f1; const_of_sum f2]] in
     [simplify_sum loops [u; t]]
  | [f1; f2] ->
     if less_than_f f2 f1 then
       [f2; f1]
     else [f1; f2]
  | (FUnion fl)::tl ->
     merge_unions loops fl (simplify_union_rec loops tl)
  | hd::tl ->
     merge_unions loops [hd] (simplify_union_rec loops tl)

and merge_unions loops fl1 fl2 =
  match fl1,fl2 with
  | [],fl | fl,[] -> fl
  | hd1::tl1, hd2::tl2 ->
     let l=simplify_union_rec loops ([hd1;hd2]) in
     match l with
     | [] -> internal_error "merge_unions" "empty list"
     | [hd] -> hd::(merge_unions loops tl1 tl2)
     | [hd1';hd2'] when hd1'=hd1 ->
        hd1::(merge_unions loops tl1 fl2)
     | [hd1';hd2'] when hd1'=hd2 ->
        hd2::(merge_unions loops fl1 tl2)       
     | _ -> internal_error "merge_unions" "wrong list size"
          
and simplify_union loops fl =
  match fl with
  | [] -> internal_error "simplify_union" "empty list"
  | [f1] -> f1 (* Might happen due to neutral element bot_f *)
  | _ ->
     let fl' = simplify_union_rec loops fl in
     match fl' with
     | [] -> internal_error  "simplify_union" "empty list"
     | [f1] -> f1
     | _ -> FUnion fl'

let simplify_annot loops (f,a) =
  match f with
  | FConst c ->
     FConst (Abstract_wcet.annot loops c a)
  | _ ->
     if f = bot_f then bot_f
     else
       FAnnot (f,a)

let simplify_power loops (f_body, f_exit, l, it) =
  match f_body, f_exit with
  | (FConst c1, FConst c2) when (not (is_symb it)) ->
     let it = int_of_symb it in
     FConst (Abstract_wcet.pow loops c1 c2 l it)
  | _,_ -> FPower (f_body, f_exit, l, it)

let rec simplify_product loops (k,f) =
  if k = 0 then
    FConst bot_wcet
  else match f with
       | FConst c ->
          FConst (prod k c)
       | FProduct (k',f') ->
          simplify_product loops (k*k',f')
       | _ -> FProduct (k,f)

(* Applies function [fct] recursively on all sub-terms of [formula]. *)            
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
    
(** Returns the simplification of formula [f]. *)
let rec simplify loops f =
  (* First, simplify sub-terms. *)
  let f' = fmap (simplify loops) f in
  match f' with
  | FConst _ | FParam _ -> f'
  | FPlus fl ->
     simplify_sum loops fl
  | FUnion fl ->
     simplify_union loops fl
  | FProduct (k,f) ->
     simplify_product loops (k,f)
  | FAnnot (f, a) ->
     simplify_annot loops (f,a)
  | FPower (f_body, f_exit, l, it) ->
     simplify_power loops (f_body, f_exit, l, it)
           
