exception Syntax_err of Location.t

open Format

let report_error loc =
  Location.print loc;
  print_string "Syntax error\n"

let wrap parsing_fun lexbuf =
  try
    let ast = parsing_fun Lexer.token lexbuf in
    Parsing.clear_parser ();
    ast
  with
    Parsing.Parse_error ->
      let loc = Location.curr lexbuf in
      raise (Syntax_err loc)

let prog = wrap Parser.prog
