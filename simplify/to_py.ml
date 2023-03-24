open Symbol
open Loops
open Wcet_formula
open Context
open Format

let id_p = ref (-1)

let rec print_eta out_f eta =
	match eta with
	| [] -> Utils.internal_error "print_eta" "eta should not be empty"
	| [x] -> fprintf out_f "%d," x
	| x::xs ->
		if List.length xs == 0 then
			fprintf out_f "%d," x
		else
			let _ = fprintf out_f "%d," x in
				print_eta out_f xs

let print_awcet out_f (eta,others)  =
	let _ = if List.length eta > 0 then print_eta out_f eta in
		fprintf out_f "%d" others

let print_lid out_f lid =
	match lid with
	| LTop -> fprintf out_f "%d" (-1)
	| LNamed str -> fprintf out_f "%d" (int_of_string str)

let print_bound out_f bound =
	match bound with
	| SInt i -> fprintf out_f "generator.ConstantBound(%d)" i
	| _ -> Utils.internal_error "print_bound" "Unsupported loop bound"

let print_term out_f term loop =
	match term.value with
	| BParam p -> fprintf out_f "((%d)*(param_%d))" term.coef (int_of_string p)
	| BConst c -> fprintf out_f "((%d)*(%d))" term.coef c

let rec print_param_bound out_f bound =
	match bound with
	| [] -> Utils.internal_error "print_param_bound" "bound should not be empty"
	| x::xs ->
		if List.length xs > 0 then
			let _ = print_term out_f x true in
			let _ = fprintf out_f " + " in
			print_param_bound out_f xs
		else
			print_term out_f x true

let rec print_terms out_f tl loop =
	match tl with
	| [] -> Utils.internal_error "print_terms" "term list sould never be empty"
	| x::xs ->
		if List.length xs == 0 then
			print_term out_f x loop
		else
			let _ = print_term out_f x loop in
			let _ = fprintf out_f " + " in
			print_terms out_f xs loop

let print_predicate out_f predicate =
	match predicate with
	| BLeq (cst,tl) ->
		let _ = fprintf out_f "%d <= "  cst in
		print_terms out_f tl false
	| BEq (cst,tl) ->
		let _ = fprintf out_f "%d == " cst in
		print_terms out_f tl false
	| BBool b ->
		if b == true then
			fprintf out_f "true"
		else
			fprintf out_f "false"

let rec print_predicates out_f predicates =
	match predicates with
	| [] -> Utils.internal_error "print_predicates" "predicates list should never be empty"
	| x::xs ->
		if List.length xs == 0 then
			print_predicate out_f x
		else
			let _ = print_predicate out_f x in
			let _ = fprintf out_f " && " in
			print_predicates out_f xs

(* Prints the formula as a C code *)
let rec f_to_py out_f f =
	match f with
	| FConst (lid,aw) ->
		let _ = fprintf out_f "generator.Constant(" in
		let _ = print_lid out_f lid in
		let _ = fprintf out_f ",[" in
		let _ = print_awcet out_f aw in
		fprintf out_f "])"
	| FPlus fl ->
		let _ = fprintf out_f "generator.Seq([" in
		let _ = print_children out_f fl in
		fprintf out_f "])"
	| FUnion fl ->
		let _ = fprintf out_f "generator.Alt([" in
		let _ = print_children out_f fl in
		fprintf out_f "])"
	| FPower (fb,_,lid,it) -> (* in practice the exit tree of the loop never appear here *)
		let _ = fprintf out_f "generator.Loop(" in
		let _ = f_to_py out_f fb in
		let _ = fprintf out_f "," in
		let _ = print_lid out_f lid in
		let _ = fprintf out_f "," in
		let _ = print_bound out_f it in
		fprintf out_f ")"
	| FPowerParam (fb,_,lid,it) -> (* same as FPower for the exit tree *)
		let _ = fprintf out_f "generator.Loop(" in
		let _ = f_to_py out_f fb in
		let _ = fprintf out_f "," in
		let _ = print_lid out_f lid in
		let _ = fprintf out_f ", generator.ParamBound('" in
		let _ = print_param_bound out_f it in
		fprintf out_f "'))"
	| FBProduct (ps,f') ->
		let _ = fprintf out_f "generator.Conditional('" in
		let _ = print_predicates out_f ps in
		let _ = fprintf out_f "'," in
		let _ = f_to_py out_f f' in
		fprintf out_f ")"
	| FProduct (k, f') ->
		Utils.internal_error "f_to_py" "TODO2"
	| FParam p -> (* We just write a variable p<id> that should be defined by the user *)
		Utils.internal_error "f_to_py" "TODO3"
	| FAnnot (f', (lid, n)) -> (* t executed at maximum n times for loop lid *)
		Utils.internal_error "f_to_py" "TODO4"

and print_children out_f children =
	match children with
	| [] -> Utils.internal_error "print_children" "children_list should never be empty"
	| x::xs ->
		if List.length xs == 0 then
			f_to_py out_f x
		else
			let _ = f_to_py out_f x in
			let _ = fprintf out_f "," in
			print_children out_f xs

let py_context out_f f =
	let _ = fprintf out_f "import sys\nsys.path.append('../code_generator')\nimport generate as generator\n" in
	let _ = fprintf out_f "formula = " in
	let _ = f_to_py out_f f in
	fprintf out_f "\ngenerator.generate(formula)\n"
