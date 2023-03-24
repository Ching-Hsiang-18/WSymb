open Symbol
open Loops
open Wcet_formula
open Context
open Format

(* Defines the type of data used *)
let static_type = "awcet"
let c_type = "awcet*"
let p_type = "int"

(* Unique awcet identifier *)
let wid_p = ref (-1)

(* Unique loop bound identifier *)
let lbid_p = ref (-1)

(* A C function to measure the execution time with high precision *)
let c_time out_f =
	let _ = fprintf out_f "#include <time.h>\n\n"
	and _ = fprintf out_f "enum { NS_PER_SECOND = 1000000000 };\n"
	and _ = fprintf out_f "void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td){\n"
	and _ = fprintf out_f "\ttd->tv_nsec = t2.tv_nsec - t1.tv_nsec;\n"
	and _ = fprintf out_f "\ttd->tv_sec  = t2.tv_sec - t1.tv_sec;\n"
	and _ = fprintf out_f "\tif (td->tv_sec > 0 && td->tv_nsec < 0){\n"
	and _ = fprintf out_f "\t\ttd->tv_nsec += NS_PER_SECOND;\n"
	and _ = fprintf out_f "\t\ttd->tv_sec--;\n"
	and _ = fprintf out_f "\t}\n\telse if (td->tv_sec < 0 && td->tv_nsec > 0) {\n"
	and _ = fprintf out_f "\t\ttd->tv_nsec -= NS_PER_SECOND;\n"
	and _ = fprintf out_f "\t\ttd->tv_sec++;\n"
	in fprintf out_f "\t}\n}\n\n"

(* Simple header print *)
let c_header out_f = fprintf out_f "loopinfo_t li = {loop_hierarchy,0};\n\nlong long wcet(%s b0, %s b1, %s b2, %s b3){\n" p_type p_type p_type p_type

(* Simple footer print *)
let c_footer out_f =
	let _ = fprintf out_f "\treturn w0->eta_count == 0 ? w0->others : w0->eta[0];\n}\n\n"
	and _ = fprintf out_f "int main(int argc, char** argv){\n"
	(*and _ = fprintf out_f "\tint b0 = atoi(argv[1]);\n"
	and _ = fprintf out_f "\tint b1 = atoi(argv[2]);\n"
	and _ = fprintf out_f "\tint b2 = atoi(argv[3]);\n"
	and _ = fprintf out_f "\tint b3 = atoi(argv[4]);\n"
	and _ = fprintf out_f "\tstruct timespec start, finish, delta;\n"*)
	and _ = fprintf out_f "\t long long w;\n"
	(*and _ = fprintf out_f "\tclock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);\n"*)
	(* Comment next line if not measuring (remove the system call duration impact) *)
	(*and _ = fprintf out_f "\tfor(int i=0;i<1000;i++)"*)
	(*and _ =*)in fprintf out_f "\t\tw = wcet(0, 1, 2, 3);\n}"
	(*and _ = fprintf out_f "\tclock_gettime(CLOCK_THREAD_CPUTIME_ID, &finish);\n"
	and _ = fprintf out_f "\tsub_timespec(start, finish, &delta);\n"
	in fprintf out_f "\tprintf(\"WCET = %s (computed in %ss)\\n\", w, (int) delta.tv_sec, delta.tv_nsec);\n\treturn 0;\n}" "%lld" "%d.%.9ld"*)

(* Prints eta to c *)
let rec eta_to_c out_f li =
	match li with
	| [] -> []
	| x::xs ->
		let _ =
			if List.length xs != 0 then
				fprintf out_f "%d," x
			else
				fprintf out_f "%d" x
		in eta_to_c out_f xs

(* parse a loop id into an integer *)
let lid_to_int lid =
	match lid with
	| LTop -> -1
	| LNamed str -> int_of_string str

(* Prints an abstract WCET statically *)
let a_to_c out_f lid aw =
	match aw with
	| (eta, others) ->
		let ilid = lid_to_int lid in
		if List.length eta == 0 then
			fprintf out_f "&(%s){ %d, %d, 0, %d}" static_type ilid 0 others
		else
			let _ = fprintf out_f "&(%s){ %d, %d, &(long long[%d]){ " static_type ilid (List.length eta) (List.length eta)
			and _ = eta_to_c out_f eta in
			fprintf out_f "}, %d}" others

(* Recursive print for lists *)
let rec fl_to_c out_f func fl =
	match fl with
	| [] -> []
	| x::xs ->
		let id = func out_f x in
		id::(fl_to_c out_f func xs)

(* Prints a list of wcets (to make arrays for sum and max) *)
let rec c_list out_f ids =
	match ids with
	| [] -> []
	| x::xs ->
		let _ =
			if List.length xs != 0 then
				fprintf out_f "w%d," x
			else
				fprintf out_f "w%d" x
		in c_list out_f xs

(* computes a string to replace eta_count (it is an upper bound), which is unknown before runtime *)
let rec eta_count_str out_f ids =
	match ids with
	| [] -> Utils.internal_error "eta_count_str" "ids should never be empty"
	| [x] -> fprintf out_f "w%d->eta_count" x
	| x::xs -> let _ =fprintf out_f "w%d->eta_count + " x in eta_count_str out_f xs

(* print a normal loop bound *)
let it_to_c out_f it =
	match it with
	| SInt i -> fprintf out_f "%d" i
	| SParam p -> fprintf out_f "p%s" p

(* print a term *)
let term_to_c out_f t =
	match t.value with
	| BConst c -> fprintf out_f "((%d) * (%d))" t.coef c
	| BParam p -> fprintf out_f "((%d) * (b%d))" t.coef (int_of_string p)

(* print a linear expression (a term list) *)
let rec terms_to_c out_f ts =
	match ts with
	| [] -> Utils.internal_error "terms_to_c" "terms list should not be empty"
	| x::xs ->
		if List.length xs > 0 then
			let _ = term_to_c out_f x
			and _ = fprintf out_f " + " in
			terms_to_c out_f xs
		else
			term_to_c out_f x

(* print a linear loop bound *)
let plb_to_c out_f le =
	incr lbid_p;
	let lbid = !lbid_p in
	let _ = fprintf out_f "\tint lb%d = " lbid in
	let _ = terms_to_c out_f le in
	let _ = fprintf out_f ";\n" in
	fprintf out_f "\tlb%d = lb%d >= 0 ? lb%d : 0;\n" lbid lbid lbid

(* print a single predicate *)
let p_to_c out_f p =
	match p with
	| BLeq (cst,tl) ->
		let _ = fprintf out_f "(%d <= " cst in
		let _ = terms_to_c out_f tl in
		fprintf out_f ")"
	| BEq (cst, tl) ->
		let _ = fprintf out_f "(%d == " cst in
		let _ = terms_to_c out_f tl in
		fprintf out_f ")"
	| _ -> Utils.internal_error "p_to_c" "p_to_c should never be called on other than == or <="

(* print predicates : only conjunctions*)
let rec ps_to_c out_f ps =
	match ps with
	| [] -> Utils.internal_error "p_to_c" "ps should never be empty"
	| x::xs ->
		if List.length xs > 0 then
			let _ = p_to_c out_f x
			and _ = fprintf out_f " && " in
			ps_to_c out_f xs
		else
			p_to_c out_f x

(* Prints the formula as a C code *)
let rec f_to_c out_f f =
	incr wid_p;
	let wid = !wid_p in
	match f with
	| FConst (lid,aw) ->
		let _ = fprintf out_f "\t%s w%d = " c_type wid
		and _ = a_to_c out_f lid aw
		and _ = fprintf out_f ";\n" in
		wid
	| FPlus fl ->
		let ids = fl_to_c out_f f_to_c fl in
		let _ = fprintf out_f "\t%s a%d[%d] = {" c_type wid (List.length ids) in
		let _ = c_list out_f ids in
		let _ = fprintf out_f "};\n\t%s w%d = sum(&a%d, %d, &li);\n" c_type wid wid (List.length ids) in
		wid
	| FUnion fl ->
		let ids = fl_to_c out_f f_to_c fl in
		(* generate temporary awcet with enough eta to compute the union *)
		let _ = fprintf out_f "\tlong long eta%d[" wid in
		let _ = eta_count_str out_f ids in
		let _ = fprintf out_f "];\n" in
		let _ = fprintf out_f "\t%s t%d = &(%s){ LOOP_TOP, " c_type wid static_type in
		let _ = eta_count_str out_f ids in
		let _ = fprintf out_f ", &eta%d, 0};\n" wid in
		(* array of alternatives awcets *)
		let _ = fprintf out_f "\t%s a%d[%d] = {" c_type wid (List.length ids) in
		let _ = c_list out_f ids in
		let _ = fprintf out_f "};\n\t%s w%d = max(&a%d, %d, t%d, &li);\n" c_type wid wid (List.length ids) wid in
		wid
	| FPower (fb,_,lid,it) -> (* in practice the exit tree of the loop never appear here *)
		let id = f_to_c out_f fb in
		let _ = fprintf out_f "\t%s w%d = iterate(w%d, %d, " c_type wid id (lid_to_int lid) in
		let _ = it_to_c out_f it in
		let _ = fprintf out_f ");\n" in
		wid
	| FPowerParam (fb,_,lid,it) -> (* same as FPower for the exit tree *)
		let id = f_to_c out_f fb in
		let _ = plb_to_c out_f it in
		let _ = fprintf out_f "\t%s w%d = iterate(w%d, %d, lb%d);\n" c_type wid id (lid_to_int lid) !lbid_p in
		wid
	| FBProduct (ps,f') ->
		let id = (f_to_c out_f f') in
		let _ = fprintf out_f "\t%s w%d = " c_type wid in
		let _ = ps_to_c out_f ps in
		let _ = fprintf out_f " ? w%d : &(%s){ -1, 0, 0, 0};\n" id static_type in
		wid
	| FProduct (k, f') ->
		let id = f_to_c out_f f' in
		let _ = fprintf out_f "\t%s w%d = int_multiply(w%d, %d)" c_type wid id k in
		wid
	| FParam p -> (* We just write a variable p<id> that should be defined by the user *)
		let _ = fprintf out_f "\t%s w%d = p%d;\n" c_type wid (int_of_string p) in
		wid
	| FAnnot (f', (lid, n)) -> (* t executed at maximum n times for loop lid *)
		let id = f_to_c out_f f' in
		(* we need enough eta space, like for unions *)
		let _ = fprintf out_f "\tlong long eta%d[%d];\n" wid n in
		let _ = fprintf out_f "\t%s w%d = annot(w%d, %d, %d, a%d);\n" c_type wid id (lid_to_int lid) n wid
		in wid

(* Main print function *)
let c_formula out_f f =
	let _ = c_header out_f
	and _ = f_to_c out_f f
	in c_footer out_f

(* Context copied from old to_c *)
let c_loop_inclusion out_f l1 l2 =
	fprintf out_f "\tif((inner == %d) && (outer == %d)) return 1;\n" (int_of_string l1) (int_of_string l2)


let c_loop_hierarchy out_f hier =
	fprintf out_f "int loop_hierarchy(int inner, int outer) {\n";
	Hashtbl.iter (fun (l1,l2) _ -> c_loop_inclusion out_f l1 l2) hier;
	fprintf out_f "\treturn 0;\n}\n"

let c_context source_name ctx =
	let basename = Filename.chop_suffix source_name Options.extension in
	let outname =
		if (!Options.out_name = "") then
			basename^".c"
    		else !Options.out_name
  	in
  	let out_ch = open_out outname in
	let out_f = formatter_of_out_channel out_ch in
	fprintf out_f "#include <pwcet.h>\n";
	c_time out_f;
	c_loop_hierarchy out_f ctx.loop_hierarchy;
	fprintf out_f "@.";
	c_formula out_f ctx.formula;
	fprintf out_f "@.";
	close_out out_ch
