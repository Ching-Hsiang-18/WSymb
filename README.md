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
(CFTree) representation from a program. 

The CFTree is a a tree-representation that looks like the Abstract Syntax
Tree (AST), but it is extracted from the binary, and does not needs source
code.

With this plugin, some static analyses that usually require the actual AST
from the source code, can be adapted to work on the CFTree instead.

The algorithm to compute the CFTree is presented in [1,2].

----
## Installation

To use this plugin, you need to have installed OTAWA v2, and the command
`otawa-config` must be in your `$PATH`.

To compile and install the plugin, type:

```
$ make
$ make install
```

----
## Usage

Once the plugin is installed, the following command will give the required
LDLIBS to use it: 

```
otawa-config otawa/display otawa/cftree --libs
```

To extract the CFTree from your program, you need to require the feature
`otawa::cftree::EXTRACTED_CFT_FEATURE`, afterwards each CFG of the workspace
will have a `CFTREE` property attached, that points to the corresponding
CFTree.

The `dumpcft` command is give as an example, to document the plugin usage.

By default, the dumpcft will take 2 or 3 parameters:

```
./dumpcft <binary file> <output header file> [<optional entry point>]
```

It will extract the CFTree in .dot format, and also produce the abstract
WCET formula as a header file. This header file can be used to instantiate
the parametric WCET at runtime.

## WCET formula instantiation example

An example is given in the example/ subdir.

The CFTreeExtractor plugin should have been compiled and installed first.

Then, compile your target program as an ARM binary (example/example.c) and
create the flowfact file using mkff. 

Launch dumpcft on the ARM binary to create the header file containing the 
parametric WCET formula (example-pwcet.h in our case) for the ARM binary.

This formula can be instantiated/evaluated without having to depend on OTAWA. 
An example is given in example/pwcet_instantiator.c, it must include
"example-pwcet.h" and "pwcet/include/pwcet-runtime.h", and be linked against 
"pwcet/libpwcet-runtime.a" (the runtime contains general formula evaluation
functions, whereas "example-pwcet.h" contains data tied to the specific
binary under analysis, produced by dumpcft)

## Simple non-parametric usage

To compute the WCET in your instantiator program, you must do:

```
    loopinfo_t li = {.hier = loop_hierarchy, .bnd = loop_bound };
    long long wcet = evaluate(&f, &li, NULL, NULL); /* the two last arguments are reserved for future use */
```

(the loop_hieararchy and loop_bound functions are defined in the header file
computed by dumpcft)

## Parametric loop bounds

To use parametric loop bounds, you can replace loop_bound by your custom
handler, so that it returns the loop bound value you want. Example:

```
#define PARAM_FLAG 0x40000000
int param_loop_bound(int loop_id) {
    int bound = loop_bounds(loop_id);
    if (bound & PARAM_FLAG) {
        int param_id = bound & ~PARAM_FLAG;
        /* return here whatever value you want for the loop bound */
        
    } else return bound;
}
```

And then, edit the flowfact file to put a loop bound of 0x4000000N (where N
is the parameter id) for each loop bound you want to be made parametric...

An example is given in example/pwcet_instantiator.c.

----
## References

[1] Ballabriga, C., Forget, J., & Lipari, G. (2018). Symbolic WCET computation. ACM Transactions on Embedded Computing Systems (TECS), 17(2), 39.
ISO 690	

[2] https://arxiv.org/abs/1709.09369





