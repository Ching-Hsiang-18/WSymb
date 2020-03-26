# CFTreeExtractor

Control Flow Tree extractor plugin for OTAWA v2

* Version 1.0.0 (for OTAWA v2), June 2019

----
## Authors

* Celestine Sauvage <celestine.sauvage@nospamgmail.com>

## Contributors

* Clement Ballabriga <Clement.Ballabriga@nospamuniv-lille.fr>

License: GPL 2.1

Please remove "nospam" from the email addresses.

----

## Introduction

CFTreeExtractor is an OTAWA v2 plugin that produces a Control Flow Tree
(CFTree) representation from binary a program.

The CFTree is a tree-representation of the binary, it can be considered
as a (enriched) binary code Abstract Syntax Tree.

The algorithm to compute the CFTree is presented in [1,2].

----
## Installation

This plugin requires [OTAWA v2](http://www.otawa.fr/). Also, the directory
`<otawa dir>/bin` (where `otawa dir` is the directory where otawa is
installed) must be in your `$PATH`. We suggest you follow [these
installation
instructions](https://gitlab.cristal.univ-lille.fr/otawa-plugins/otawa-doc),
and stop at the end of section "Setting the environment".

To compile and install the plugin:

```
$ make
```

----
## Usage

You can use the command `dumpcft` to produce the CFT from a binary code:

```
./dumpcft <binary file> <output header file> [<optional entry point>]
```

*Warning*: `binary file` must be a binary for an instruction set
supported by Otawa. Typically, an ARM binary. See the `example`
sub-directory for a `Makefile` example.

`dumpcft` outputs the CFTree in a `.dot` format, and also produces the
abstract WCET formula as a header file. This header can be used to
instantiate the parametric WCET (either before, or during, run-time).

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

## Formula instantiation

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

In the example, procedure `param_loop_bound` relates loop identifiers to
loop bound. Edit `example.ff` to write `0x4000000N` (where N is the
parameter identifier) instead of `??` for each loop bound you want to
turn parametric. The parameter identifier is used in function
`param_value` to instantiate the parameter (i.e. provide its value).

Note that you can instead replace procedure `param_loop_bound` by your
custom handler if you wish.

----
## References

[1] Ballabriga, C., Forget, J., & Lipari, G. (2018). Symbolic WCET computation. ACM Transactions on Embedded Computing Systems (TECS), 17(2), 39.
ISO 690	

[2] https://arxiv.org/abs/1709.09369





