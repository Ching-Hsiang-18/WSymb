open Abstract_wcet
open Wcet_formula
open Simplify   

let usage = "Usage: simplify <source-file>"

(* Simplify a list of formulas and pretty-print the results. *)          
let simplify_prog formulas =
  List.iter
    (fun (f,loops) ->
      let f' = simplify loops f in
      Format.fprintf Format.std_formatter "%a %a@."
        Wcet_formula.pp f'
        Loops.pp_hier loops
    )
    formulas

(* Process file named [source_name]. Results is printed on standard output. *)  
let anonymous source_name =
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
  simplify_prog prog

(* Do the job. *)  
let _ =
  try
    Arg.parse Options.options anonymous usage
  with
  | Parse.Syntax_err _ | Lexer.Error _ -> ()
