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

(** Abstract WCET definitions and operators. *)

open Utils
open Loops
   
type annot = loop_id * int

(* Might switch to symb_int later *)           
type wcet = int

(* We assume that for any [(wl, last): multi_wcet], [wl] is sorted
   decreasingly and [last] is smaller than every value in [wl]. [last]
   is implicitely repeated indefinitely. *)
type multi_wcet = wcet list * wcet

type abstract_wcet = loop_id * multi_wcet

let bot_wcet = (LTop, ([],0))

(** Returns the highest WCET of [(wl, last)] *)
let hd (wl, last) =
  if wl = [] then last (* Because last is repeated indefinitely *)
  else List.hd wl

(** Returns the given multi_wcet without its highest WCET *)  
let tl (wl, last) =
  if wl = [] then ([], last) (* Because last is repeated indefinitely *)
  else (List.tl wl,last)  

(** Inserts [w] in [(wl, last)] keeping the result sorted decreasingly. *)  
let insert w (wl, last) =
  let cmp = fun x y -> -(compare x y) in
  if w > last then
    (List.merge cmp wl [w], last) (* Keep wl sorted decreasingly. *)
  else if w = last then
    (wl, last)
  else (List.merge cmp wl [last], w) (* w is the new last, ex-last goes into wl *)

(* Sum on multi_wcet *)
let rec sum_mwcet w1 w2  =
  (* Basically, a point-by-point addition. *)
  match w1, w2 with
  | ([], last1), ([], last2) -> ([], last1+last2)
  | _,_ ->
     (* Thanks to the definition of hd, we don't need to worry about
        (w1, w2) not having the same length. *)
     insert ((hd w1) + (hd w2)) (sum_mwcet (tl w1) (tl w2))

(** Returns the sum of two abstract wcets. *)    
let sum loops (l1,w1) (l2,w2) =
  (Loops.glb loops l1 l2, sum_mwcet w1 w2)

(* Union on multi_wcet. *)  
let rec union_mwcet w1 w2 =
  (* Basically, a point-by-point max. *)
  match w1, w2 with
  | ([], last1), ([], last2) -> ([], Stdlib.max last1 last2)
  | ([], last), w | w, ([], last) ->
     if (last >= hd w) then (* last is greater than all of w1 *)
       ([], last)
     else insert (hd w) (union_mwcet ([], last) (tl w))
  | _,_ ->
     let max, min =
       if hd w1 >= hd w2 then
         w1,w2
       else
         w2,w1
     in
     (* Recursion: step to the next element of max, keep min as is. *)
     insert (hd max) (union_mwcet (tl max) min)

(** Returns the union of two abstract wcets. *)     
let union loops (l1,w1) (l2,w2) =
  (Loops.glb loops l1 l2, union_mwcet w1 w2)

(* Multiply [(wl,last)] by [k]. *)  
let prod_mwcet k (wl,last) =
  (List.map (fun w -> w*k) wl, k*last)

(** Returns the product of [(l,w):abstract_wcet] by k. *)
let prod k (l,w) =
  (l, prod_mwcet k w)

(* Auxilliary function for annotations computation. Returns the [n]
   first values of multi_wcet [w]. *)
let rec keep_first n w =
  if n = 0 then ([],0)
  else
    insert (hd w) (keep_first (n-1) (tl w))

(** Computes the application of annotation [(l',it)] on abstract wcet [(l, w)] *)
let annot loops (l, w) (l',it) =
  (Loops.glb loops l l', keep_first it w)

(* Several auxilliary operations on loops. *)  

(* Returns the sum of the first [k] values of [w]. *)
let rec val_sum_k_first k w =
  if k=0 then 0
  else
    (hd w) + val_sum_k_first (k-1) (tl w)

(* Returns the abstract wcet consisting of an infinite repetition of
   the sum of the first [k] values of [w]. *)  
let wl_sum_k_first k w =
  ([], val_sum_k_first k w)

(* Drops the first [k] values of [w]. *)
let rec ktl k w =
  if k = 0 then w
  else tl (ktl (k-1) w)

(* Each value of the result is the sum of [k] successive values of [w]. *)  
let rec pack k w =
  (*
   * FIX for instruction cache effect modeling, which crashes in case of loops with 0 iteration
   * when k = 0 (iteration number = 0), thus the WCET of the loop can be and should be 0, remove the next three lines to revert
   *)
  if k = 0 then
    ([], 0)
  else
    match w with
    | ([], last) -> ([], val_sum_k_first k ([],last))
    | (wl,_) -> insert (val_sum_k_first k w) (pack k (ktl k w))

(** Returns the abstract wcet corresponding to a loop whose body has
   abstract WCET [(h_body,w_body)], and whose exit node has abstract
   WCET [(h_exit,w_exit)]. The loop is iterated [it] times and has
   header [l]. *)
let pow loops (h_body,w_body) (h_exit,w_exit) l it =
  if h_body = l then
    (h_exit,sum_mwcet (wl_sum_k_first it w_body) w_exit)
  else
    (Loops.glb loops h_body h_exit, sum_mwcet (pack it w_body) w_exit)

(* Pretty printing *)
  
open Format
      
let pp_annot out_f (loop, it) =
  fprintf out_f "(%a,%d)" pp_loop_id loop it
   
let pp out_f (loop, (wl,last)) =
  if (wl <> []) then
    fprintf out_f "(%a;{%a,%d})" pp_loop_id loop
      (pp_print_list
         ~pp_sep:(fun out_f () -> pp_print_text out_f ",")
         (fun out_f w -> fprintf out_f "%d" w))
      wl last
  else
    fprintf out_f "(%a;{%d})" pp_loop_id loop last
