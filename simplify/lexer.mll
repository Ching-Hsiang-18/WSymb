(* ----------------------------------------------------------------------------
 * Copyright (C) 2020, Université de Lille, Lille, FRANCE
 * 
 * This file is part of WSymb.
 * 
 * WSymb is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation ; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * WSymb is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program ; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * ---------------------------------------------------------------------------- *)


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
| '-' {BMINUS}
| '|' {PIPE}
| '.' {DOT}
| '*' {BMULT}
| '=' {EQUALS}
| "\u{2264}" {LEQUALS}
| '&' {AND}
| '^' {CIRC}
| '(' {LPAR}
| ')' {RPAR}
| '{' {LCURLBRACKET}
| '}' {RCURLBRACKET}
| ',' {COMMA}
| ';' {SCOL}
| "b:" {BPARAM}
| "p:" {PARAM}
| "l:" {LOOP_ID}
| "true" {TRUE}
| "false" {FALSE}
| "_C" {INC}
| ['0'-'9']+
    {INT (int_of_string (Lexing.lexeme lexbuf)) }
| "__top" {TOP}
| "loops:" {LOOPS}
| "endl" {ENDLH}
| ['A'-'Z' 'a'-'z'] ['A'-'Z' 'a'-'z' '_' '0'-'9'] * { IDENT (Lexing.lexeme lexbuf) }
| eof { EOF }
| _ { raise (Error (Location.curr lexbuf))}
