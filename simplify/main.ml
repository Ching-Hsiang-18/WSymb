open Abstract_wcet
open Wcet_formula
open Simplify   
   
let c = FConst (LNamed "l1", ([5;3],2))
let c2 = FConst (LNamed "l1", ([4],1))      
let p = FParam "p"
let p2 = FParam "p2"      
let f1 = FPlus [FProduct (2, c); c]
let f2 = FPlus [c; FProduct (2, c)]
let f3 = FPlus [c; c; c]
let f4 = FPlus [c; p; c]
let f5 = FUnion [c; c; c]
let f6 = FUnion [c; p; c]
let f7 = FPlus [f4; f4]
let f8 = FPlus [f5; f5]
let f9 = FPlus [f6; f6]       
let f10 = FProduct (3, FProduct(2, p))
let f11 = FProduct (3, FProduct(2, c))
let f12 = FProduct (3, FProduct(2, FPlus [c;p]))
let f13 = FPlus [FProduct (3, p); p]
let f14 = FPlus [FProduct (3, FUnion [p;c]); FUnion [c;p]]        
let f15 = FUnion [FPlus [c; p]; FPlus [c2; p]]
let f16 = FUnion [p2; p; c2; c]
let f17 = FAnnot (c, (LNamed "l1",2)) 
let f18 = FPlus [f17; p]
let f19 = FPower (c, c2, LNamed "l1", 4)
let f20 = FPower (c, c2, LNamed "l2", 4)        
        
let formulas = [f3; f4; f5; f6; f7; f8; f9; f10; f11; f12; f13; f14; f15; f16; f17; f18; f19; f20]

let usage = "Usage: simplify <source-file>"
             
let simplify_prog formulas =
  List.iter
    (fun f ->
      let f' = simplify f in
      if (!Options.debug) then
        Format.fprintf Format.std_formatter "%a@[<hov 2> => %a@]@.@."
          Wcet_formula.pp f Wcet_formula.pp f'
      else
        Format.fprintf Format.std_formatter "%a@." Wcet_formula.pp f'
    )
    formulas

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
  
let _ =
  try
    Arg.parse Options.options anonymous usage
  with
  | Parse.Syntax_err _ | Lexer.Error _ -> ()
