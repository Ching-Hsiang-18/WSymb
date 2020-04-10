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

type param = string
(** Symbolic integer is either a constant or a symbol *)   
type symb_int = SInt of int | SParam of param

(* Returns true if sint is a symbol. *)          
let is_symb sint =
  match sint with
  | SInt _ -> false
  | SParam _ -> true

(* Returns the constant corresponding to [sint]. Fails if [sint] is a symbol. *)
let int_of_symb sint =
  match sint with
  | SInt i -> i
  | SParam _ -> Utils.internal_error "int_of_symb" "cannot be applied to param"
                                      
open Format

let pp_param out_f p =
  fprintf out_f "p:%s" p
       
let pp_symb_int out_f sint =
  match sint with
  | SInt i ->
     fprintf out_f "%d" i
  | SParam p ->
     pp_param out_f p
                                          
