open Abstract_wcet

type t =
  FConst of abstract_wcet
  | FParam of param    
  | FPlus of t list
  | FUnion of t list
  | FPower of t * t * loop_id * symb_int
  | FAnnot of t * annot
  | FProduct of int * t

let bot_f = FConst bot_wcet


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
     fprintf out_f "@[<hov 2>(%a, %a, %a)^%a@]" pp f1 pp f2 pp_loop l pp_symb_int it
  | FAnnot (f, a) ->
     fprintf out_f "@[<hov 2>%a|%a@]" pp f pp_annot a
  | FProduct (k, f) ->
     fprintf out_f "@[<hov 2>%d.(%a)@]" k pp f
