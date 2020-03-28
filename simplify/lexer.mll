{
  open Parser

  exception Error of Location.t
     
  (* Update line number for location info *)
  let incr_line lexbuf =
    let pos = lexbuf.Lexing.lex_curr_p in
    lexbuf.Lexing.lex_curr_p <-
      { pos with
        Lexing.pos_lnum = pos.Lexing.pos_lnum + 1;
        Lexing.pos_bol = pos.Lexing.pos_cnum;
      }
}

let newline = ('\010' | '\013' | "\013\010")
let blank = [' ' '\009' '\012']

rule token = parse
| newline { incr_line lexbuf; token lexbuf }
| blank + { token lexbuf }
| 'U' {UNION}            
| '+' {PLUS}
| '|' {PIPE}
| '.' {DOT}
| '^' {CIRC}
| '(' {LPAR}
| ')' {RPAR}
| '{' {LCURLBRACKET}
| '}' {RCURLBRACKET}
| ',' {COMMA}
| ';' {SCOL}
| ['0'-'9']+
    {INT (int_of_string (Lexing.lexeme lexbuf)) }
| "__top" {TOP}
| ['A'-'Z' 'a'-'z'] ['A'-'Z' 'a'-'z' '_' '0'-'9'] * { IDENT (Lexing.lexeme lexbuf) }
| eof { EOF }
| _ { raise (Error (Location.curr lexbuf))}
