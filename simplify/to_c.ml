open Symbol
open Loops
open Wcet_formula
open Context
open Format

let c_array pp_elem ty_string out_f array =
  let length = List.length array in
  if length = 0 then
    pp_print_text out_f "NULL"
  else
    fprintf out_f "@[<hov 2>(%s[%d])@ {%a}@]"
      ty_string
      (List.length array)
      (fun out_f ->
        pp_print_list
          ~pp_sep:(fun out_f () -> fprintf out_f ",@ ")
          pp_elem out_f)
      array
   
let c_symb_int out_f sint =
  match sint with
  | SInt  i -> pp_print_int out_f i
  | SParam p -> pp_print_text out_f p
   
let c_loop_bound out_f lname bound =
  fprintf out_f "case %s: return %a;@ " lname c_symb_int bound
  
let c_loop_bounds out_f bounds =
  fprintf out_f "@[<v 2>int loop_bounds(int loop_id) {@ ";
  fprintf out_f "@[<v 2>switch(loop_id) {@ ";
  Hashtbl.iter (c_loop_bound out_f) bounds;
  fprintf out_f "default: abort();";
  fprintf out_f "@]@ }@]@.}@."

let c_loop_inclusion out_f l1 l2 =
  fprintf out_f "if((inner == %d) && (outer == %d)) return 1;@ "
    (int_of_string l1) (int_of_string l2)
  
let c_loop_hierarchy out_f hier =
  fprintf out_f "@[<v 2>int loop_hierarchy(int inner, int outer) {@ ";
  Hashtbl.iter (fun (l1,l2) _ -> c_loop_inclusion out_f l1 l2) hier;
  fprintf out_f "return 0;";
  fprintf out_f "@]@.}@."

let c_loopid out_f lid =
  match lid with
  | LNamed n -> pp_print_int out_f (int_of_string n)
  | LTop -> pp_print_string out_f "LOOP_TOP"
  
let c_wlist out_f (wl, last) =
  fprintf out_f "%d,@ %a,@ %d" (List.length wl) (c_array pp_print_int "long long") wl last

let c_null_wcet out_f () =
  fprintf out_f "@[<hov 2>{-1,@ 0,@ NULL,@ 0}@]"
  
let c_awcet out_f (lid, wl) =
  fprintf out_f "@[<hov 2>{%a,@ %a}@]"
    c_loopid lid
    c_wlist wl                          

let c_annot out_f (lid, k) =
  fprintf out_f ".ann={%a,%d}" c_loopid lid k
  
let rec c_formula_operands out_f fl =
  c_array c_formula_rec "formula_t" out_f fl
  
and c_formula_rec out_f f =
  fprintf out_f "@[<hov 2>{";
  begin
    match f with
    | FConst aw ->
       fprintf out_f "KIND_CONST,@ 0,@ {0},@ %a,@ NULL" c_awcet aw
    | FParam p ->
       fprintf out_f "KIND_AWCET,@ %d,@ {0},@ %a,@ NULL" (int_of_string p) c_null_wcet ()
    | FPlus fl ->
       fprintf out_f "KIND_SEQ,@ 0,@ {%d},@ %a, %a"
         (List.length fl)
         c_null_wcet ()
         c_formula_operands fl
    | FUnion fl ->
       fprintf out_f "KIND_ALT,@ 0,@ {%d},@ %a, %a"
         (List.length fl)
         c_null_wcet ()
         c_formula_operands fl
    | FPower (fbody, fexit, lid, it) ->
       begin
         match it with
         | SInt i ->
            fprintf out_f "KIND_LOOP,@ 0,@ {%d},@ %a, %a"
              i c_null_wcet () c_formula_operands [fbody]
         | SParam p ->
            fprintf out_f "KIND_LOOP,@ %d,@ {0},@ %a, %a"
              (int_of_string p)
              c_null_wcet ()
              c_formula_operands [fbody]
       end
    | FAnnot (f, a) ->
       fprintf out_f "KIND_ANN,@ 0,@ {%a},@ %a,@ %a"
         c_annot a c_null_wcet () c_formula_operands [f]
    | FProduct (k, f) ->
       fprintf out_f "KIND_INTMULT,@ 0,@ {%d},@ %a,@ %a"
         k c_null_wcet () c_formula_operands [f]
  end;
  fprintf out_f "}@]"
  
let c_formula out_f f =
  fprintf out_f "@[<hov 2>formula_t f@ =@ %a;@]@ " c_formula_rec f

let c_context source_name ctx =
  let basename = Filename.chop_suffix source_name Options.extension in
  let outname =
    if (!Options.out_name = "") then
      basename^".h"
    else !Options.out_name
  in
  let out_ch = open_out outname in
  let out_f = formatter_of_out_channel out_ch in
  c_loop_bounds out_f ctx.loop_bounds;
  fprintf out_f "@.";
  c_loop_hierarchy out_f ctx.loop_hierarchy;
  fprintf out_f "@.";
  c_formula out_f ctx.formula;
  fprintf out_f "@.";
  close_out out_ch
