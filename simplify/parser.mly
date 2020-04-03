/* ---------------------------------------------------------------------------- */
/* Copyright (C) 2020, Université de Lille, Lille, FRANCE */

/* This file is part of CFTExtractor. */

/* CFTExtractor is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU Lesser General Public License */
/* as published by the Free Software Foundation ; either version 2 of */
/* the License, or (at your option) any later version. */

/* CFTExtractor is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY ; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU */
/* Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public */
/* License along with this program ; if not, write to the Free Software */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 */
/* USA */
/* ---------------------------------------------------------------------------- */

%{
  open Utils
  open Loops
  open Abstract_wcet
  open Wcet_formula
%}

%token <string> IDENT
%token <int> INT
%token PIPE
%token DOT
%token UNION PLUS
%token CIRC
%token LPAR RPAR LCURLBRACKET RCURLBRACKET COMMA SCOL TOP INC PARAM
%token LOOPS ENDLH
                                                    
%token EOF

%start prog
%type <Context.t list> prog

%nonassoc COMMA CIRC SCOL

// Not sure who should have the highest priority
%left PLUS
%left UNION

%right DOT
%left PIPE
    
%%
                              
prog:
  context_list EOF { List.rev $1 }

context_list:
  context {[$1]}
| context_list context {$2::$1}

context:
formula hierarchy {
  let f,bl = $1 in
  Context.new_ctx f $2 (Loops.bounds_from_list bl)
}

hierarchy:
  LOOPS loop_inc_list ENDLH {$2}

loop_inc_list:
    {Loops.new_hierarchy ()}
    | loop_inc_list loop_inc {
          let inner,outers = $2 in
          Loops.add_inclusions $1 inner outers;
          $1
        }

loop_inc:
  IDENT INC ident_list SCOL { ($1, $3)}

ident_list:
  IDENT {[$1]}
| ident_list IDENT {$2::$1}

// Restrict to binary arity operators.
// Can't think of a simple solution to directly parse a full list of operands.    
formula:
    const { (FConst $1, []) }
  | param { (FParam $1, []) }
  | formula PLUS formula
    {
      let (f1,bl1),(f2,bl2) = $1,$3 in
      (FPlus [f1; f2], bl1@bl2)
    }
  | formula UNION formula
    {
      let (f1,bl1),(f2,bl2) = $1,$3 in
      (FUnion [f1; f2], bl1@bl2)
    }
  | LPAR formula COMMA formula COMMA loop_id RPAR CIRC sint
    {
      let (f1,bl1),(f2,bl2) = $2,$4 in
      (FPower (f1, f2, $6, $9), ($6,$9)::bl1@bl2)
    }
  | formula PIPE annot
    {
      let f, bl = $1 in
      (FAnnot (f, $3), bl)
    }
  | INT DOT formula
    {
      let f, bl = $3 in
      (FProduct ($1, f), bl)
    }
  | LPAR formula RPAR {$2}
      
const:
LPAR loop_id SCOL LCURLBRACKET wcet_list RCURLBRACKET RPAR
{ let last = List.hd $5 in
  let wlist = List.rev (List.tl $5) in
  ($2, (wlist, last)) }

wcet_list:
INT {[$1]}
| wcet_list COMMA INT {$3::$1}

loop_id:
TOP {LTop}
| IDENT { LNamed $1}

annot:
LPAR loop_id COMMA INT RPAR { ($2, $4) }

sint:
INT {SInt $1}
| param { SParam $1}

param:
  PARAM INT { string_of_int $2 }
