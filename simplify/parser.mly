%{
    open Abstract_wcet
    open Wcet_formula
%}

%token <string> IDENT
%token <int> INT
%token PIPE
%token DOT
%token UNION PLUS
%token CIRC
%token LPAR RPAR LCURLBRACKET RCURLBRACKET COMMA SCOL TOP

%token EOF

%start prog
%type <Wcet_formula.t list> prog

%nonassoc COMMA CIRC SCOL

// Not sure who should have the highest priority
%left PLUS
%left UNION

%right DOT
%left PIPE
    
%%
                              
prog:
  formula_list EOF { List.rev $1}

formula_list:
  formula {[$1]}
| formula_list formula {$2::$1}

formula:
    const { FConst $1 }
  | IDENT { FParam $1 }
  // Can't think of a simple solution to directly parse a full list of operands
  | formula PLUS formula { FPlus [$1; $3] }
  | formula UNION formula { FUnion [$1; $3] }
  | LPAR formula COMMA formula COMMA loop_id RPAR CIRC INT
      { FPower ($2, $4, $6, $9) }
  | formula PIPE annot
      { FAnnot ($1, $3)}
  | INT DOT formula
      { FProduct ($1, $3)}
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
