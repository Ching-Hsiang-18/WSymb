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

(** The abstract WCET formula type. *)                                      
type t =
  FConst of abstract_wcet
  | FParam of param    
  | FPlus of t list
  | FUnion of t list
  | FPower of t * t * loop_id * symb_int
  | FAnnot of t * annot
  | FProduct of int * t

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
  | FAnnot (_,(_,it)) ->
     it
  | FProduct (_,f') ->
     multi_wcet_size_bound f'
    
(* Pretty printing *)
              
open Format
   
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
  | FAnnot (f, a) ->
     fprintf out_f "@[<hov 2>%a|%a@]" pp f pp_annot a
  | FProduct (k, f) ->
     fprintf out_f "@[<hov 2>%d.(%a)@]" k pp f
