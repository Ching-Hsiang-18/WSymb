open Abstract_wcet
open Wcet_formula
open Simplify
open Context   
          
let compile source_name contexts =
  if !Options.to_c then
    if List.length contexts <> 1 then
      raise (Arg.Bad "Compilation to C code applies only to a single formula.")
    else
      let ctx = List.hd contexts in
      let f' = simplify ctx.loop_hierarchy ctx.formula in
      let ctx' = new_ctx f' ctx.loop_hierarchy ctx.loop_bounds in
      To_c.c_context source_name ctx'
  else
    let out_f =
    if (!Options.out_name = "") then
      Format.std_formatter
    else
      let out_ch = open_out !Options.out_name in
      Format.formatter_of_out_channel out_ch
    in

    List.iter
      (fun ctx ->
        let f' = simplify ctx.loop_hierarchy ctx.formula in
        Format.fprintf out_f "%a %a@."
          Wcet_formula.pp f'
          Loops.pp_hier ctx.loop_hierarchy
      )
      contexts
  
(* Process file named [source_name]. Results is printed on standard output. *)  
let anonymous source_name =
  if Filename.check_suffix source_name Options.extension then
    begin
      Location.input_name := source_name;
      let lexbuf = Lexing.from_channel (open_in source_name) in
      Location.init lexbuf source_name;
      let prog =
        try
          Parse.prog lexbuf
        with (Lexer.Error loc) | (Parse.Syntax_err loc) as exc ->
                                  Parse.report_error loc;
                                  raise exc
      in
      compile source_name prog 
    end
  else
    raise (Arg.Bad ("Can only process *.pwf files"))

(* Do the job. *)  
let _ =
  try
    Arg.parse Options.options anonymous Options.usage
  with
  | Parse.Syntax_err _ | Lexer.Error _ -> ()
  | exc -> raise exc
