(** License goes here *)

type param = string
type symb_int = SInt of int | SParam of param
type loop_id = LNamed of string | LParam of param | LTop
type annot = loop_id * symb_int
type wcet = int
type multi_wcet = wcet list * wcet (* Separate the last, implicitely infinitely repeated *)
type abstract_wcet = loop_id * multi_wcet

let bot_wcet = (LTop, ([],0))

(* TODO *)             
let upper_bound l1 l2 =
  l1
             
let hd (wl, last) =
  if wl = [] then last
  else List.hd wl

let tl (wl, last) =
  if wl = [] then (wl, last)
  else (List.tl wl,last)  

let insert w (wl, last) =
  let cmp = fun x y -> -(compare x y) in
  if w > last then
    (List.merge cmp wl [w], last)
  else if w = last then
    (wl, last)
  else (List.merge cmp wl [last], w)
  
(* TODO? *)  
let nth w n =
  List.nth w n
             
let rec sum_mwcet w1 w2  =
  match w1, w2 with
  | ([], last1), ([], last2) -> ([], last1+last2)
  | _,_ ->
     insert ((hd w1) + (hd w2)) (sum_mwcet (tl w1) (tl w2))

let sum (l1,w1) (l2,w2) =
  (upper_bound l1 l2, sum_mwcet w1 w2)
    
let rec union_mwcet w1 w2 =
  match w1, w2 with
  | ([], last1), ([], last2) -> ([], max last1 last2)
  | ([], last), w | w, ([], last) ->
     if (last >= hd w) then ([], last)
     else insert (hd w) (union_mwcet ([], last) (tl w))
  | _,_ ->
     let max, min =
       if hd w1 >= hd w2 then
         w1,w2
       else
         w2,w1
     in
     insert (hd max) (union_mwcet (tl max) min)
     
let union (l1,w1) (l2,w2) =
  (upper_bound l1 l2, union_mwcet w1 w2)

let prod_mwcet k (wl,last) =
  (List.map (fun w -> w*k) wl, k*last)

let prod k (l,w) =
  (l, prod_mwcet k w)
  
let rec less_than_awcet w1 w2 =
  match (w1,w2) with
    | ([], last1), ([], last2) -> last1 < last2
    | _,_ ->
       let hd1, hd2= (hd w1), (hd w2) in
       if (hd w1) <> (hd w2) then
         hd1 < hd2
       else
         less_than_awcet (tl w1) (tl w2)
     
open Format

let pp_param out_f p =
  pp_print_text out_f p
   
let pp_loop out_f l =
  match l with
  | LNamed n ->
     fprintf out_f "\"%s\"" n
  | LParam p ->
     pp_param out_f p
  | LTop ->
     pp_print_text out_f "__top"

let pp_symb_int out_f sint =
  match sint with
  | SInt i ->
     fprintf out_f "%d" i
  | SParam p ->
     pp_param out_f p

let pp_annot out_f (loop, it) =
  fprintf out_f "(%a,%a)" pp_loop loop pp_symb_int it
   
let pp out_f (loop, (wl,last)) =
  fprintf out_f "(%a,{%a,%d})" pp_loop loop
    (pp_print_list
       ~pp_sep:(fun out_f () -> pp_print_text out_f ",")
       (fun out_f w -> fprintf out_f "%d" w))
    wl last
