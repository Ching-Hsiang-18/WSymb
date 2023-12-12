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

(* let rec c_condition_print out_f condition =
  fprintf out_f "@[<hov 2>{";
  begin
    match condition with
    | BConst c -> fprintf out_f "BOOL_CONST,@ %d,@ 0,@ {NULL},@ NULL" c
    | BParam p -> fprintf out_f "BOOL_PARAM,@ %d,@ 0,@ {NULL},@ NULL"
      (int_of_string p)
    | BPlus bl -> fprintf out_f "BOOL_PLUS,@ 0,@ {%d},@ {NULL},@
    (formula_t[%d]){%a}"
      (List.length bl)
      (List.length bl)
      (fun out_f ->
        pp_print_list
	~pp_sep:(fun out_f () -> fprintf out_f ",@ ")
	c_condition_print out_f
      ) bl
    | BMinus bl -> fprintf out_f "BOOL_MINUS,@ 0,@ {%d},@ {NULL},@
    (formula_t[%d]){%a}"
      (List.length bl)
      (List.length bl)
      (fun out_f ->
        pp_print_list
	~pp_sep:(fun out_f () -> fprintf out_f ",@ ")
	c_condition_print out_f
      ) bl
    | BLeq (c,be) -> fprintf out_f "BOOL_LEQ,@ 0,@ {2},@ {NULL},@
    (formula_t[2]){";
      c_condition_print out_f (BConst c);
      fprintf out_f ", ";
      c_condition_print out_f be;
      fprintf out_f "}"
    | BEq (c,be) -> fprintf out_f "BOOL_LEQ,@ 0,@ {2},@ {NULL},@
    (formula_t[2]){";
      c_condition_print out_f (BConst c);
      fprintf out_f ", ";
      c_condition_print out_f be;
      fprintf out_f "}"
    | BAnd bl -> fprintf out_f "BOOL_AND,@ 0,@ {%d},@ {NULL},@ (formula_t[%d])%a"
      (List.length bl)
      (List.length bl)
      (fun out_f ->
        pp_print_list
	~pp_sep:(fun out_f () -> fprintf out_f ",@ ")
	c_condition_print out_f
      ) bl

  end;
  fprintf out_f "}@]"
 *)

let print_term out_f t =
  match t.value with
  | BConst c -> fprintf out_f "{BOOL_CONST,@ %d,@ %d}" t.coef c
  | BParam p -> fprintf out_f "{BOOL_PARAM,@ %d,@ %d}" t.coef
  (int_of_string p)

let rec print_terms out_f tl =
  match tl with
  | [] -> Utils.internal_error "print_terms" "term list should never be empty"
  | [t] -> print_term out_f t
  | t :: ts -> 
    print_terms out_f [t];
    if (List.length ts) > 0 then
      begin
      fprintf out_f ", ";
      print_terms out_f ts
      end

(** Print loop bounds *)
let rec print_loop_bound out_f loop_bound =
  begin
    fprintf out_f "(condition_t[1]) {BOOL_BOUND,@ 0,@ %d,@ (term_t[%d]){" (List.length loop_bound) (List.length loop_bound);
    print_terms out_f loop_bound;
    fprintf out_f "}}"
  end

let rec print_conditions out_f conditions =
  match conditions with
  | [] -> Utils.internal_error "print_conditions" "condition list should never be empty"
  | [c] ->
    begin
    match c with
    | BLeq (cst,tl) ->
      fprintf out_f "{BOOL_LEQ,@ %d,@ %d,@ (term_t[%d]){" cst (List.length
      tl) (List.length tl);
      print_terms out_f tl;
      fprintf out_f "}}"
    | BEq (cst,tl) ->
      fprintf out_f "{BOOL_EQ, @ %d,@ %d,@ (term_t[%d]){" cst (List.length
      tl) (List.length tl);
      print_terms out_f tl;
      fprintf out_f "}}"
    | _ -> Utils.internal_error "print_conditions" "Unsupported condition type"
    end
  | c :: cs ->
    print_conditions out_f [c];
    if (List.length cs) > 0 then
      begin
      fprintf out_f ", ";
      print_conditions out_f cs
      end

let c_conditions_print out_f conditions =
  fprintf out_f "@[<hov 2>{BOOL_CONDITIONS,@ 0, @ {%d},@ {0},@ NULL,@
  (condition_t[%d]){" (List.length conditions) (List.length conditions);
  print_conditions out_f conditions;
  fprintf out_f "}}@]"

let c_array_with_boolean pp_elem ty_string conditions out_f array =
  let length = List.length array in
  if length = 0 then
    pp_print_text out_f "NULL"
  else
    fprintf out_f "@[<hov 2>(%s[%d])@ {"(* "%a,@ %a}@]" *)
    ty_string
    ((List.length array)+1);
    c_conditions_print out_f conditions;
    fprintf out_f ",@ %a}@]" 
    (fun out_f ->
      pp_print_list
        ~pp_sep:(fun out_f () -> fprintf out_f ",@ ")
	pp_elem out_f
    ) array 
  
let c_loop_bound out_f lname bound =
  match bound with
  | SInt i ->
     fprintf out_f "case %s: return %d;@ " lname i
  | SParam _ -> ()
  
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

let c_aw_placeholder out_f f =
  match f with
  | FConst _ -> Utils.internal_error "c_aw_placeholder" "f should not be const or param"
  | _ ->
     let eta_count = if multi_wcet_size_bound f > 0 then multi_wcet_size_bound f else 1 in
     fprintf out_f "@[<hov 2>{-1,@ %d,@ (long long[%d]){0},@ 0}@]" eta_count eta_count
  
let c_awcet out_f (lid, wl) =
  fprintf out_f "@[<hov 2>{%a,@ %a}@]"
    c_loopid lid
    c_wlist wl                          

let c_annot out_f (lid, k) =
  fprintf out_f ".ann={%a,%d}" c_loopid lid k

let rec c_boolean_operands conditions out_f fl =
  c_array_with_boolean c_formula_rec "formula_t" conditions out_f fl

and c_formula_operands out_f fl =
  c_array c_formula_rec "formula_t" out_f fl

and c_formula_rec out_f f =
  fprintf out_f "@[<hov 2>{";
  begin
    match f with
    | FConst aw ->
       fprintf out_f "KIND_CONST,@ 0,@ {0},@ %a,@ NULL" c_awcet aw
    | FParam p ->
       fprintf out_f "KIND_AWCET,@ %d,@ {0},@ %a,@ NULL" (int_of_string p) c_aw_placeholder f
    | FPlus fl ->
       fprintf out_f "KIND_SEQ,@ 0,@ {%d},@ %a, %a"
         (List.length fl)
         c_aw_placeholder f
         c_formula_operands fl
    | FUnion fl ->
       fprintf out_f "KIND_ALT,@ 0,@ {%d},@ %a, %a"
         (List.length fl)
         c_aw_placeholder f
         c_formula_operands fl
    | FPower (fbody, _, lid, it) ->
       begin
         match it with
         | SInt i ->
            fprintf out_f "KIND_LOOP,@ 0,@ {%a},@ %a, %a"
              c_loopid lid c_aw_placeholder f c_formula_operands [fbody]
         | SParam p ->
            fprintf out_f "KIND_LOOP,@ %d,@ {%a},@ %a, %a"
              (int_of_string p)
              c_loopid lid
              c_aw_placeholder f
              c_formula_operands [fbody]
       end
    | FPowerParam (fb,_,lid,it) ->
      begin
        fprintf out_f "KIND_PARAM_LOOP,@ 0,@ {%a},@ {0},@ %a,@ "
          c_loopid lid c_formula_operands [fb];
	  print_loop_bound out_f it
      end
    | FAnnot (f', a) ->
       fprintf out_f "KIND_ANN,@ 0,@ {%a},@ %a,@ %a"
         c_annot a c_aw_placeholder f c_formula_operands [f']
    | FProduct (k, f') ->
       fprintf out_f "KIND_INTMULT,@ 0,@ {%d},@ %a,@ %a"
         k c_aw_placeholder f c_formula_operands [f']
    | FBProduct (es,f') ->
       fprintf out_f "KIND_BOOLMULT,@ 0,@ {2},@ {0}, @ ";
       c_boolean_operands es out_f [f']
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
