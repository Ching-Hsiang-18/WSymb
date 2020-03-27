open Abstract_wcet
open Wcet_formula
open Simplify   
   
let c = FConst (LNamed "l1", ([5;3],2))
let p = FParam "p"      
let f1 = FPlus [FProduct (2, c); c]
let f2 = FPlus [c; FProduct (2, c)]
let f3 = FPlus [c; c; c]
let f4 = FPlus [c; p; c]
let f5 = FUnion [c; c; c]
let f6 = FUnion [c; p; c]
let f7 = FPlus [f4; f4]
let f8 = FPlus [f5; f5]
let f9 = FPlus [f6; f6]       
       
let formulas = [f3; f4; f5; f6; f7; f8; f9]


let _ =
  List.iter
    (fun f -> let f' = simplify f in
              Format.fprintf Format.std_formatter "%a@ @[<hov 2>=>@ %a@]@.@."
                Wcet_formula.pp f Wcet_formula.pp f')
    formulas

