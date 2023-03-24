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

open Utils
open Symbol
open Loops
open Abstract_wcet

(** The abstract boolean expression type *)
type bval =
  BConst of int (* constant *)
  | BParam of bparam (* a function parameter *)
type term = {coef : int; value : bval}
type b =
  BLeq of int * term list (* inequation *)
  | BEq of int * term list (* equation *)
  | BBool of bool (* constant value *)

(*type b = 
  BConst of int (* a constant *)
  | BParam of bparam (* a function parameter *)
  | BPlus of b list (* a sum of elements *)
  | BMinus of b list (* a difference of elements *)
  | BLeq of int * b (* an inequation *)
  | BEq of int * b (* an equation *)
  | BAnd of b list (* a conjunction of expressions *)
*)

(** The abstract WCET formula type. *)                                      
type t =
  FConst of abstract_wcet
  | FParam of param    
  | FPlus of t list
  | FUnion of t list
  | FPower of t * t * loop_id * symb_int
  | FPowerParam of t * t * loop_id * term list
  | FAnnot of t * annot
  | FProduct of int * t
  | FBProduct of b list * t

let bot_f = FConst bot_wcet

(** Computes an upper bound to the size of the [multi_wcet]
   corresponding to formula [f] ([last] excluded). *)
let rec multi_wcet_size_bound f =
  match f with
  | FParam _ -> 1
  | FConst (_,(wl,_)) -> List.length wl
  | FPlus flist ->
     List.fold_left (fun mx f -> max mx (multi_wcet_size_bound f)) 1 flist
  | FUnion flist ->
     List.fold_left (fun sum f ->  sum+(multi_wcet_size_bound f)) 0 flist
  | FPower (fbody,fexit,_,it) ->
     let bound_body = multi_wcet_size_bound fbody in
     let bound_exit = multi_wcet_size_bound fexit in
     begin
       match it with
       | SInt i ->
          max (int_of_float (ceil ((float_of_int bound_body) /. (float_of_int bound_exit))))
            bound_exit
       | SParam _ -> max bound_body bound_exit
     end
  | FPowerParam (fbody,fexit,_,it) ->
     let bound_body = multi_wcet_size_bound fbody in
     let bound_exit = multi_wcet_size_bound fexit in
        max bound_body bound_exit
  | FAnnot (_,(_,it)) ->
     it
  | FProduct (_,f') ->
     multi_wcet_size_bound f'
  | FBProduct (_,f') ->
     multi_wcet_size_bound f'
    
(* Pretty printing *)
              
open Format

(** pretty print bval elements *)
let pp_bval out_f v =
  match v with
  | BConst c -> fprintf out_f "%d" c
  | BParam p -> fprintf out_f "b:%s" p

(** pretty print right part of expression *)
let rec pp_expr out_f tl first =
  match tl with
  | [] -> Utils.internal_error "pp_expr" "Right part of an expression should never be empty"
  | [h] ->
    if h.coef >= 0 then
      begin
      if h.coef == 1 then
        begin
	if first == false then fprintf out_f " + ";
	pp_bval out_f h.value
	end
      else
        begin
        if first == false
	then fprintf out_f " + %d * " h.coef
	else fprintf out_f " %d * " h.coef;
	pp_bval out_f h.value
	end
      end
    else
      begin
      if h.coef == -1 then
        begin
        fprintf out_f " - ";
        pp_bval out_f h.value
	end
      else
        begin
        fprintf out_f " - %d * " h.coef;
	pp_bval out_f h.value
	end
      end;
  | h :: t ->
    pp_expr out_f [h] first;
    if (List.length t) > 0 then
      pp_expr out_f t false

let rec pp out_f f =
  match f with
  | FConst w ->
     Abstract_wcet.pp out_f w
  | FParam p ->
     pp_param out_f p
  | FPlus fl ->
     fprintf out_f "@[<hov 2>(%a)@]"
       (fun out_f ->
         pp_print_list
           ~pp_sep:(fun out_f () -> fprintf out_f "@ +@ ")
           pp out_f
       ) fl
  | FUnion fl ->
     fprintf out_f "@[<hov 2>(%a)@]"
       (fun out_f ->
         pp_print_list
           ~pp_sep:(fun out_f () -> fprintf out_f "@ U@ ")
           pp out_f
       ) fl
  | FPower (f1, f2, l, it) ->
     fprintf out_f "@[<hov 2>(%a, %a, %a)^%a@]" pp f1 pp f2 pp_loop_id l pp_symb_int it
  | FPowerParam (fb,fe,l,it) ->
     begin
        fprintf out_f "@[<hov 2>(%a, %a, %a)^(" pp fb pp fe pp_loop_id l;
        pp_expr out_f it true;
	fprintf out_f ")@]"
     end
  | FAnnot (f, a) ->
     fprintf out_f "@[<hov 2>%a|%a@]" pp f pp_annot a
  | FProduct (k, f) ->
     fprintf out_f "@[<hov 2>%d.(%a)@]" k pp f
  | FBProduct (e,f) ->
     fprintf out_f "@[<hov 2>((%a)*(%a))@]" pp_bel e pp f

(** pretty print for boolean expressions list *)
and pp_bel out_f bel =
  match bel with
  | [] -> Utils.internal_error "pp_bel" "Boolean expression list should never be empty"
  | [x] -> pp_be out_f x
  | h :: t ->
    pp_be out_f h;
    if (List.length t) > 0 then
      fprintf out_f " & ";
      pp_bel out_f t

(** pretty print for boolean expression *)
and pp_be out_f be =
  match be with
  | BLeq (cst,e) ->
    fprintf out_f "%d \u{2264}" cst;
    pp_expr out_f e true
  | BEq (cst,e) ->
    fprintf out_f "%d =" cst;
    pp_expr out_f e true
  | BBool b ->
    let b_str = if b then "true" else "false" in
    fprintf out_f "%s" b_str

(*and pp_be out_f be =
  match be with
  | BConst c -> fprintf out_f "@[<hov 2>%d@]" c
  | BParam p -> fprintf out_f "@[<hov 2>%s@]" p
  | BPlus bl ->
    fprintf out_f "@[<hov 2>%a@]"
      (fun out_f ->
        pp_print_list
	  ~pp_sep:(fun out_f () -> fprintf out_f "@ +@ ")
	  pp_be out_f
      ) bl
  | BMinus bl ->
    fprintf out_f "@[<hov 2>%a@]"
      (fun out_f ->
        pp_print_list
	  ~pp_sep:(fun out_f () -> fprintf out_f "@ -@ ")
	  pp_be out_f
      ) bl
  | BLeq (c,e) ->
    fprintf out_f "@[<hov 2>%d \u{2264} %a@]" c pp_be e
  | BEq (c,e) ->
    fprintf out_f "@[<hov 2>%d = %a@]" c pp_be e
  | BAnd bl ->
    fprintf out_f "@[<hov 2>%a@]"
      (fun out_f ->
        pp_print_list
	~pp_sep:(fun out_f () -> fprintf out_f "@ &@ ")
	pp_be out_f
      ) bl *)
