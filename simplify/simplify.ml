(* ----------------------------------------------------------------------------
 * Copyright (C) 2020, Université de Lille, Lille, FRANCE
 *
 * This file is part of WSymb.
 *
 * WSymb is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation ; either version 2 of
 * the License, or (at your option) any later version.
 *
 * WSymb is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program ; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *---------------------------------------------------------------------------- *)

open Symbol
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

(** Extracts parameter id from the string of the parameter *)
let param_id bparam =
  int_of_string bparam

(** Comparison between two terms of boolean formula *)
let bval_diff t1 t2 =
  match t1.value with
  | BConst x ->
    begin
      match t2.value with
      | BConst y ->
        x - y
      | _ -> -1
    end
  | BParam x ->
    begin
      match t2.value with
      | BConst _ -> 1
      | BParam y ->
        let xint = param_id x and yint = param_id y in
	xint - yint
    end

(** Sorting boolean expression terms *)
let sort_boolean_terms tl =
  List.sort bval_diff tl

(** lowest parameter id in a term list *)
let rec lowest_id tl =
  match tl with
  | [] -> -1
  | x::xs ->
    match x.value with
    | BConst _ -> -1
    | BParam p ->
      let pid = param_id p and lid = lowest_id xs in
      if lid = -1 || pid < lid then
        pid
      else
        lid

let rec lowest_id_expressions el =
  match el with
  | [] -> -1
  | x::xs ->
    begin
      match x with
      | BLeq (_,tl) | BEq (_,tl) -> 
        let id = lowest_id tl and ido = lowest_id_expressions xs in
        if ido = -1 || id < ido then
          id
        else
          ido
      | _ -> -1
    end

(** constant in a list of terms *)
let rec const_term tl =
  match tl with
  | [] -> 0
  | x::xs ->
    begin
      match x.value with
      | BConst c -> c
      | _ -> const_term xs
    end

(** comparison of boolean expressions *)
let expression_diff e1 e2 =
  match e1 with
  | BBool _ -> -1
  | BLeq (_,tl1) | BEq (_,tl1) ->
    begin
      match e2 with
      | BBool _ -> 1
      | BLeq (_,tl2) | BEq (_,tl2) ->
        let diff = lowest_id tl1 - lowest_id tl2 in
          if diff == 0 then
            let len = List.length tl1 - List.length tl2 in
	    if len == 0 then
              let c1 = const_term tl1 and c2 = const_term tl2 in
	      c1 - c2
	    else
	      len
          else
	    diff
      end

(** sorting boolean conjunctions *)
let sort_expressions cs =
  List.sort expression_diff cs



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
  | FBProduct (e1,f1), FBProduct (e2,f2) ->
    let lid1 = lowest_id_expressions e1 and lid2 = lowest_id_expressions e2
    in
    if lid1 = lid2 then
      if List.length e1 < List.length e2 then
        true
      else
        false
    else
      if lid1 < lid2 then
        true
      else
        false
  | FPowerParam (_, _, _,tl1), FPowerParam (_,_,_,tl2) ->
    let lid1 = lowest_id tl1 and lid2 = lowest_id tl2 in
    if lid1 = lid2 then
      if List.length tl1 < List.length tl2 then
        true
      else
        false
    else
      if lid1 < lid2 then
        true
      else
        false
  | FConst _, _ -> true
  | FBProduct _, FProduct _ | FBProduct _, FPlus _ | FBProduct _, FUnion _| FBProduct _, FPower _| FBProduct _, FAnnot _ | FBProduct _, FParam _ -> true
  | FProduct _, FPlus _ | FProduct _, FUnion _ | FProduct _, FPower _ | FProduct _, FAnnot _ | FProduct _, FParam _ -> true
  | FPlus _, FUnion _ | FPlus _, FPower _ | FPlus _, FAnnot _ | FPlus _, FParam _ -> true
  | FUnion _, FPower _ | FUnion _, FAnnot _ | FUnion _, FParam _ -> true
  | FPower _, FAnnot _ | FPower _, FParam _ -> true
  | FPowerParam _, FParam _ | FPowerParam _, FPlus _ | FPowerParam _, FUnion _ | FPowerParam _, FPower _ | FPowerParam _, FAnnot _ | FPowerParam _, FProduct _ | FPowerParam _, FBProduct _-> true
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
  | FConst _ -> Utils.internal_error "const_of_prod" "f should not be const"
  | _ -> 1

let rec simplify_product loops (k,f) =
  if k = 0 then
    FConst bot_wcet
  else match f with
       | FConst c ->
          FConst (prod k c)
       | FProduct (k',f') ->
          simplify_product loops (k*k',f')
       | _ -> FProduct (k,f)

(* check if a list is included in an other list *)
let rec check_included bl1 bl2 =
  match bl1 with
  | [] -> true
  | x::xs ->
    let res = (List.mem x bl2) in
    if res = true then
      check_included xs bl2
    else
      false

(** retrieve a list containing element of a not in b *)
let rec intersection_complement l1 l2 =
  match l1 with
  | [] -> []
  | x::xs ->
    if List.mem x l2 then
      intersection_complement xs l2
    else
      x :: intersection_complement xs l2

(** Comparison between terms of two inequations, not easy to implement *)
let opposed_term_leq t1 t2 =
  if t1.coef <> -t2.coef then
    false
  else
    match t1.value, t2.value with
    | BParam p1, BParam p2 ->
      if p1 = p2 then
        true
      else
        false
    | BConst c1, BConst c2 ->
      let c1b = if t1.coef >= 0 then c1 else c1-1
      and c2b = if t2.coef >= 0 then c2 else c2-1 in
      if c1b = c2b then
        true
      else
        false
    | _,_ -> false

(** find a term from bval in a term list *)
let rec find_by_bval bv tl =
  match tl with
  | [] -> []
  | x::xs ->
    match bv,x.value with
    | BParam _, BParam _ ->
      if x.value = bv then
        [x]
      else
        find_by_bval bv xs
    | BConst _, BConst _ -> [x]
    | _,_ -> find_by_bval bv xs

(** check that two linear expression are opposed *)
let rec opposed_le l1 l2 =
  match l1 with
  | [] -> true
  | x::xs ->
    begin
      let tl = find_by_bval x.value l2 in
      match tl with
      | [t] ->
        if opposed_term_leq x t then
          opposed_le xs l2
        else
          false
      | _ -> false
    end

(** Comparison between two equations *)
let opposed e1 e2 =
  match e1,e2 with
  | BLeq (c1,le1), BLeq (c2,le2) ->
    (* comparison between the two lists of terms*)
    if List.length le1 <> List.length le2 then
      false
    else
      opposed_le le1 le2
  | _,_ -> false

(** Comparison between equation and list*)
let rec exists_opposed e es =
  match es with
  | [] -> false
  | x::xs ->
    if opposed e x then
      true
    else
      exists_opposed e xs

(** check if conjunctions are opposed *)
let rec opposed_conjunctions es1 es2 =
  match es1 with
  | [] -> true
  | x::xs ->
    if exists_opposed x es2 then
      opposed_conjunctions xs es2
    else
      false

(* Assumes that [List.length fl] >= 2 *)       
let rec simplify_sum_rec loops fl =
  match fl with
  | [] | [_] -> Utils.internal_error "simplify_sum_rec" "wrong list size"
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
  | [FBProduct (e1,f1); FBProduct (e2,f2)] -> (* dictributivity: e*t+e*t1
  -> e*(t+t1 *)
     if check_included e1 e2 && check_included e2 e1 then
       [FBProduct(e1, simplify_sum loops [f1;f2])]
     else (* distributivity: ek*t + (ek&el)*t2 -> ek*(t+el*t2) for the two
     next cases*)
       if check_included e1 e2 then
         [FBProduct (e1, simplify_sum loops [f1;FBProduct ((intersection_complement e2 e1),f2)])]
       else
         if check_included e2 e1 then
	   [FBProduct (e2, simplify_sum loops [FBProduct ((intersection_complement e1 e2),f1);f2])]
	 else
	   if opposed_conjunctions e1 e2 && f1 = f2 then
	     [f1]
	   else
             [FBProduct (e1,f1); FBProduct (e2,f2)]
  | [f1; f2] when (term_of_prod f1=term_of_prod f2 && term_of_prod f1 <> None) ->
     let t = match term_of_prod f1 with
       | Some t' -> t'
       | None -> Utils.internal_error "simplify_sum_rec" "cannot be none"
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
     | [] -> Utils.internal_error "merge_sums" "empty list"
     | [hd] -> hd::(merge_sums loops tl1 tl2)
     | [hd1';hd2'] when hd1'=hd1 ->
        hd1::(merge_sums loops tl1 fl2)
     | [hd1';hd2'] when hd1'=hd2 ->
        hd2::(merge_sums loops fl1 tl2)       
     | _ -> Utils.internal_error "merge_sums" "wrong list size"
          
and simplify_sum loops fl =
  match fl with
  | [] -> Utils.internal_error "simplify_sum" "empty list"
  | [f1] -> f1 (* Might happen due to neutral element bot_f *)
  | _ ->
     let fl' = simplify_sum_rec loops fl in
     match fl' with
     | [] -> Utils.internal_error  "simplify_sum" "empty list"
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
  | FConst _ -> Utils.internal_error "const_of_sum" "f should not be const"
  | _ -> FConst bot_wcet

open Printf
(* Assumes that [List.length fl] >= 2 *)          
let rec simplify_union_rec loops fl =
  match fl with
  | [] | [_] -> Utils.internal_error "simplify_union_rec" "wrong list size"
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
  | [FBProduct (e1,f1);FBProduct (e2,f2)] -> (* distributivity: e*tUe*t1
  -> e*(tUt1) *)
     if check_included e1 e2 && check_included e2 e1 then
       [FBProduct(e1, (*simplify_union loops*) FUnion [f1;f2])] (* There is a bug here when simplifying the resulting union, something goes wrong and removes a part of the formula ?!?*)
     else (* distributivity: ek*t + (ek&el)*t2 -> ek*(t+el*t2) for the two
     next cases*)
       if check_included e1 e2 then
         [FBProduct (e1, simplify_union loops [f1;FBProduct ((intersection_complement e2 e1),f2)])]
       else
         if check_included e2 e1 then
	   [FBProduct (e2, simplify_union loops [FBProduct ((intersection_complement e1 e2),f1);f2])]
	 else
	   if opposed_conjunctions e1 e2 && f1 = f2 then
	     [f1]
	   else
             [FBProduct (e1,f1);FBProduct (e2,f2)]
  | [f1; f2] when (term_of_sum f1=term_of_sum f2 && term_of_sum f1 <> None) ->
     let t = match term_of_sum f1 with
       | Some t' -> t'
       | None -> Utils.internal_error "simplify_union_rec" "cannot be none"
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
     | [] -> Utils.internal_error "merge_unions" "empty list"
     | [hd] -> hd::(merge_unions loops tl1 tl2)
     | [hd1';hd2'] when hd1'=hd1 ->
        hd1::(merge_unions loops tl1 fl2)
     | [hd1';hd2'] when hd1'=hd2 ->
        hd2::(merge_unions loops fl1 tl2)       
     | _ -> Utils.internal_error "merge_unions" "wrong list size"
          
and simplify_union loops fl =
  match fl with
  | [] -> Utils.internal_error "simplify_union" "empty list"
  | [f1] -> f1 (* Might happen due to neutral element bot_f *)
  | _ ->
     let fl' = simplify_union_rec loops fl in
     match fl' with
     | [] -> Utils.internal_error  "simplify_union" "empty list"
     | [f1] -> f1
     | _ -> FUnion fl'

let simplify_annot loops (f,a) =
  match f with
  | FConst c ->
     FConst (Abstract_wcet.annot loops c a)
  | FBProduct (e,t) -> (* annotation: ann(e*t,a) -> e* ann(t,a) *)
     FBProduct (e,FAnnot(t,a))
  | _ ->
     if f = bot_f then bot_f
     else
       FAnnot (f,a)

let simplify_power loops (f_body, f_exit, l, it) =
  match f_body, f_exit with
  | (FConst c1, FConst c2) when (not (is_symb it)) ->
     let it = int_of_symb it in
     FConst (Abstract_wcet.pow loops c1 c2 l it)
  | FBProduct (e,t), _ -> (* loop: (e*t,exit,b^it -> exit + e*(t, bot_f,
  b)^it *)
     if f_exit = bot_f then
       FBProduct(e, FPower(t, bot_f, l, it))
     else
       FPlus [f_exit; FBProduct(e,FPower(t, bot_f, l, it))]
  | _,_ -> FPlus [f_exit; FPower (f_body, bot_f, l, it)]

(* special treatment for parametric loops : can't simplify static bound *)
let simplify_power_param loops (f_body, f_exit, l, it) =
  match f_body, f_exit with
  | FBProduct (e,t), _ -> (* loop: (e*t,exit,b^it -> exit + e*(t, bot_f,
  b)^it *)
     if f_exit = bot_f then
       FBProduct(e, FPowerParam(t, bot_f, l, it))
     else
       FPlus [f_exit; FBProduct(e,FPowerParam(t, bot_f, l, it))]
  | _,_ -> FPlus [f_exit; FPowerParam (f_body, bot_f, l, it)]


open Printf
(** Simplifies boolean product *)
let simplify_bproduct loops (bl,t) =
  if t = bot_f then
    bot_f (* multiplication: e*BOTTOM -> BOTTOM *)
  else
    match bl with
    | [BBool e] ->
      if e = false then
        bot_f (* multiplication: false*t -> BOTTOM if e == false *)
      else
        t (* multiplication: e*t -> t if e == true *)
    | _ ->
      begin
      match t with
        | FBProduct (bl1,t1) ->
           let bl' = sort_expressions bl in
           if check_included bl bl1 && check_included bl1 bl then
             FBProduct (bl',t1) (* multiplication:  e*(e*t)) -> e*t *)
           else
             let bl'' = sort_expressions (bl' @ bl1) in
             FBProduct(bl'', t1) (* associativity: ek*(el*t) -> (ek&el)*t *)
        | _ ->
          let bl' = sort_expressions bl in
          FBProduct (bl',t)
      end

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
  | FPowerParam (f1, f2, l, it) ->
     FPowerParam (fct f1, fct f2, l, it)
  | FAnnot (f, a) ->
     FAnnot (fct f, a)
  | FProduct (k, f) ->
     FProduct (k, fct f)
  | FBProduct (bl, f) ->
     FBProduct (bl, fct f)

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
  | FPowerParam (fb, fe, l, it) ->
     simplify_power_param loops (fb, fe, l, it)
  | FBProduct (bl, t) ->
     simplify_bproduct loops (bl, t)
