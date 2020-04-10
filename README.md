# WSymb

WSymb: Symbolic Worst-Case Execution Time (WCET) computation.

* Version 1.0.0 (for OTAWA v2), April 2020

----
## Authors

* Clement Ballabriga <Clement.Ballabriga@nospamuniv-lille.fr>
* Julien Forget <Julien.Forget@nospamuniv-lille.fr>
* Celestine Sauvage <celestine.sauvage@nospamgmail.com>

License: GPL 2.1 or higher.

Please remove "nospam" from the email addresses.

----

## Introduction

WSymb is a WCET analysis tool. Its main specificity is that, instead of
a constant WCET, it computes a *WCET formula*, where symbols (or
parameters) can correspond to various kind of values unnkown at analysis
time. The formula can later be *instanciated*, when parameter values are
known. For more details on the underlying theory, check out the papers
[1,2].

WSymb consists of several separate parts:

* `dumpcft` is an OTAWA v2 plugin that, starting from a binary program:
  1. Computes the corresponding Control Flow Tree (CFTree);
  2. Computes the WCET formula for this CFT;
  3. Outputs the formula as a `.pwf` file.

* `simplify/swimplify` is a WCET formula simplification tool:
  1. It simplifies a `.pwf` using various arithmetic properties of WCET
     formulas (distributivity, commutativity, etc);
  2. The simplified formula can either be translated into another `.pwf` file,
     or translated into C code to instanciate the formula.

* The instantiation of the WCET formula relies on an API documented in
  `pwcet/include/pwcet-tuntime.h`.

A complete example covering all these different parts is provided in
directory `example`.

----

## dumpcft

`dumpcft` requires [OTAWA v2](http://www.otawa.fr/). Also, the
directory `<otawa dir>/bin` (where `otawa dir` is the directory where
otawa is installed) must be in your `$PATH`. We suggest you follow
[these installation
instructions](https://gitlab.cristal.univ-lille.fr/otawa-plugins/otawa-doc),
and stop at the end of section "Setting the environment".

To compile and install the plugin:

```
$ make
```

Usage:
```
./dumpcft <binary file> <output header file> [<optional entry point>]
```

*Warning*: `binary file` must be a binary for an instruction set
supported by Otawa. Typically, an ARM binary. See the `example`
sub-directory for a `Makefile` example.

`dumpcft` outputs the CFTree in both a `.dot` and a `.pwf` format.

----

## swymplify

* Dependencies: the following packages are required to compile this tool: `ocaml`,
`ocamlbuild`, `make`.

* Compilation: go to directory `simplify` and type `make`.

* Execution: `./swymplify -help`

----

## WCET formula instantiation

An example of formula instantiation is provided in the `example/`
sub-directory. The compilation process, detailed in the `Makefile`,
involves the following steps:

1. Compile your C program as an ARM binary: `make example`
2. Create the flowfact file: first `mkff example > example.ff`. Then,
   for now, edit `example.ff` and replace the `??` fields by the actual
   loop bounds (later, we will detail how to use loop bound parameters
   instead)
3. Create the header file containing the parametric WCET formula for the
   ARM binary: `make example-pwcet.h`
4. Compile the formula instantiator: `make pwcet_instantiator`
5. Instantiate the formula: `./pwcet_instantiator`
6. If everything worked correctly, the execution should print the same
   WCET as the one computed by Otawa at step 3, during the CFT creation.

Note that `pwcet/include/pwcet-runtime.h` contains generic formula
evaluation functions. On the contrary, `example-pwcet.h`, which is
produced by `dumpcft`, contains code specifically related to the binary
under analysis.

The file `example/pwcet_instantiator.c` provides an example of WCET
formula instantiation. The following line provides the information on
loops hierarchy and loop bounds:

```
    loopinfo_t li = {.hier = loop_hierarchy, .bnd = loop_bound };
```

The following line evaluates the formula `f`, using the loops
information `li`:

```
    long long wcet = evaluate(&f, &li, NULL, NULL); /* the two last arguments are reserved for future use */
```

### Non-parametric loop bounds

To compute the non-parametric WCET:

1. Specify loop bound values in `example.ff` (in place of `??`);
2. `make`
3. `./pwcet_instantiator`

### Parametric loop bounds

(We plan to improve this part in future works).

Edit `example.ff` to write `0x40000001` instead of `??` for each loop
bound you want to turn parametric.

In `pwcet_instantiator.c`, procedure `param_valuation` relates parameter
identifiers to their values. In this example, the WCET is successively
instantiated with values ranges from 0 to 20 for parameter 1. Parameter
identifiers are visible in the `.pwf` file.

----
## References

[1] Ballabriga, C., Forget, J., & Lipari, G. (2018). Symbolic WCET computation. ACM Transactions on Embedded Computing Systems (TECS), 17(2), 39.
ISO 690	

[2] https://arxiv.org/abs/1709.09369

