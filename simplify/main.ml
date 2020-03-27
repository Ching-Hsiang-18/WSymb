open Abstract_wcet
open Wcet_formula
open Simplify   
   
let c = FConst (LNamed "l1", ([5;3],2))
let p = FParam "p"      
let f1 = FPlus [FProduct (SInt 2, c); c]
let f2 = FPlus [c; FProduct (SInt 2, c)]
let f3 = FPlus [c; c; c]
let f4 = FPlus [c; p; c]
       
let _ =
  let simp = simplify f4 in
  Format.fprintf Format.std_formatter "%a@," Wcet_formula.pp simp
