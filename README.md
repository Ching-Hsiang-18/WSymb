# CFTreeExtractor

Control Flow Tree extractor plugin for OTAWA v2

* Version 1.0.0 (for OTAWA v2), June 2019

----
## Authors

* Celestine Sauvage <celestine.sauvage@nospamgmail.com>

## Contributors

* Clement Ballabriga <Clement.Ballabriga@nospamuniv-lille.fr>

License: GPL 2.0

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

----
## References

[1] Ballabriga, C., Forget, J., & Lipari, G. (2018). Symbolic WCET computation. ACM Transactions on Embedded Computing Systems (TECS), 17(2), 39.
ISO 690	

[2] https://arxiv.org/abs/1709.09369





