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

type loop_id = LNamed of string | LTop

(** If [Hashtbl.mem hier (n1, n2)], then loop [n1] is 
   immediately contained in loop [n2]. *) 
(* Could be optimized, but not sure we need to, for now. *)
type hierarchy = ((string * string), unit) Hashtbl.t

type bounds = (string, symb_int) Hashtbl.t                    

let lname_from_lid lid =                
  match lid with
  | LNamed n -> n
  | LTop -> Utils.internal_error "bounds_from_list" "cannot set an iteration bound on top"
            
let new_hierarchy () : hierarchy = Hashtbl.create 42
                                 
(* Loop [inner] is included into all loops of list [outer]. *)                                      
let add_inclusions hier inner outers =
  List.iter (fun outer ->
      Hashtbl.add hier (lname_from_lid inner,lname_from_lid outer) ())
    outers
                                      
(* Returns true if [l1] is immediately contained in [l2]. *)
let imm_contained hier l1 l2 =
  match (l1, l2) with
  | _, LTop -> true
  | LTop, _ -> false
  | LNamed n1, LNamed n2 ->
     Hashtbl.mem hier (n1,n2)

let min hier l1 l2 =
  if (imm_contained hier l2 l1) then
    l2
  else l1

(** Greatest lower bound.*)
(* Actually, returns the min of l1 l2, since it should be the same as
   the glb due to program structures.*)
let glb hier l1 l2 =
  min hier l1 l2

let new_bounds () : bounds = Hashtbl.create 42

let add_bound bounds lname bound =
  Hashtbl.add bounds lname bound

let bounds_from_list l =
  let bounds = new_bounds () in
  List.iter (fun (lid,bound) ->
      let lname = lname_from_lid lid in
      Hashtbl.add bounds lname bound) l;
  bounds
  
open Format

let pp_loop_id out_f l =
  match l with
  | LNamed n ->
     fprintf out_f "l:%s" n
  | LTop ->
     pp_print_text out_f "__top"
  
let pp_hier out_f hier =
  fprintf out_f "@[<hov 2>loops:@ ";
  Hashtbl.iter
    (fun (n1,n2) _ ->
      fprintf out_f "@[<hov 2>%a _C %a;@ @]"
        pp_loop_id (LNamed n1) pp_loop_id (LNamed n2))
    hier;
  fprintf out_f "endl@]"
