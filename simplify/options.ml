let version = "1.0.0"
let debug = ref false
          
let options = [
    "-debug", Arg.Set debug, "Run in debug mode";
    "-version", Arg.Unit (fun () -> print_endline version), "Print version"
  ]
