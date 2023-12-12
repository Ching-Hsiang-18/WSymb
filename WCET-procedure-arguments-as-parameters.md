# WCET analysis with procedure arguments as parameters

----

## Authors

* Clement Ballabriga <Clement.Ballabriga@nospamuniv-lille.fr>
* Julien Forget <Julien.Forget@nospamuniv-lille.fr>
* Sandro Grebant <Sandro.Grebant@nospamuniv-lille.fr>

Please remove "nospam" from the email addresses.

----

## Introduction

This document describes a parametric WCET analysis toolset, which
analyzes the binary code of a procedure to produce a WCET formula that
represents the WCET of the procedure as a function of its
arguments. It implements the approach presented in [1].

----

## 	Requirements

Required packages:
- WSymb, cloned from its
  [git](https://gitlab.cristal.univ-lille.fr/otawa-plugins/polymalys);
- Otawa for WSymb (see instructions in the README.md of WSymb);
- Polymalys, cloned from its
  [git](https://gitlab.cristal.univ-lille.fr/otawa-plugins/WSymb);
- Otawa for Polymalys (see instructions in the README.md of Polymalys);
- Basic compilation tools (make, gcc, g++, as well as the arm
  versions).


In the following:
- ``<otawa-poly>`` denotes the installation directory of Otawa for Polymalys;
- ``<otawa-wsymb>`` denotes the instaltation directory of Otawa for WSymb;
- ``<polymalys>`` denotes the cloned repository of Polymalys;
- ``<wsymb>`` denotes the cloned repository of WSymb.

----

## Installing

### Switching between Otawa versions

In the following, we use two different versions of Otawa: one for
WSymb, another one for Polymalys. We provide a script to switch
between the environments associated to the two different
versions. First, update the paths declared at the beginning of
`do_in.sh`.

Then, the script is run as follows:

```shell
./do_in.sh <polymalys|wsymb> <my_command>
```

You must choose either ``polymalys`` or ``wsymb``, and
``<my_command>`` is the command to run in the corresponding
environment.


In the following, the required version is specified when necessary.

### Polymalys

Compile Polymalys (using Otawa for Polymalys):

```shell
./do_in.sh polymalys make -C <polymalys> install; make -C <polymalys>/tests poly
```

### WSymb

Compile and install WSymb (using Otawa for WSymb):

```shell
./do_in.sh wsymb make -C <wsymb> install; make -C <wsymb>/simplify; make -C <wsymb>/libpwcet
```

----

## Usage

### Program to analyze

First, compile the program to analyze as a standalone arm 32-bit
binary file. For instance, you can use the folowing command:

```shell
arm-none-eabi-gcc -O0 -g -nostdinc -nostdlib -mtune=cortex-a8 -mfpu=neon -mfloat-abi=hard -o <program>.elf <program>.c
```

Once compiled, you must specify the loop bounds of the program (or
the functions of interest) to Otawa.  First, generate the flow facts
file wih Otawa (any of the two Otawa versions should produce the same
result):

```shell
./do_in.sh wsymb <otawa-wsymb>/bin/mkff <programme>.elf [function] > <programme>.ff
```

Specifying the function is optional. If the function is not specified,
the complete binary file will be analyzed by ``mkff``. The generated
``.ff`` and ``.elf`` files should be in the same directory for the
next steps.

In the ``.ff`` file, for each loop, the '?' symbol must be replaced
by the loop bound.

Note that the ``.ff`` file can also contain "multi-branch" lines. Each
of these lines indicates that Otawa was unable to find the addresses
to which the program jumps (for instance, for switch-cases). Thus,
Otawa asks you to replace '?' with the address to which the program
jumps.  In general, if you do not fill these addresses, Otawa will be
unable to compute the WCET of the program. However, it is still
possible to estimate the WCET for the parts of the program that do not
contain such "multi-branches".

### Abstract interpretation with Polymalys

In this section, we use the Otawa for Polymalys.

Run the analysis:

```shell
./do_in.sh polymalys <polymalys>/tests/poly <program>.elf <function>
```

The analysis creates two files in the working directory:
``constraints.csv`` and ``loop_bounds.csv``. They contain the input
conditionals respectively for ``if-then-else`` and loops.

### Analyse de WCET avec WSymb

In this section, we use Otawa for WSymb.

Generate the WCET formula:

```shell
./do_in.sh wsymb <wsymb>/dumpcft <program>.elf <programme>.pwf <function>
```

The analysis outputs the formula in file ``<program>.pwf``.

Then, we  simplify the formula. There are three possible outputs:
1. Text formula (``.pwf``, by default, more human-readable);
2. Formula instantiator as a C program (recommended for easy testing);
3. Formula as a python object (used to produce a low execution time
   instantiator).

#### Text formula

Run the following command:

```shell
<wsymb>/simplify/swymplify <program>.pwf > <program>_simplified.pwf
```

The simplified formula is in ``<programme>_simplified.pwf``.

#### With instantiation library

Produce the C code of the instantiator:
```shell
<wsymb>/simplify/swymplify -c -i -o <program>_simplified.c <program>.pwf
```

Compile the instantiator:

```shell
gcc -O3 -g -Wall -static -march=native -I<wsymb>/libpwcet/ -o <program>_simplified <program>_simplified.c <wsymb>/libpwcet/libpwcet.a
```

(Optional) Compile the instantiator for execution on a ARM
architecture:

```shell
arm-none-eabi-gcc -O3 -g -Wall -static -march=native -I<wsymb>/libpwcet/ -o <program>_simplified <program>_simplified.c <wsymb>/libpwcet/libpwcet_arm.a
```

Run the instatiator as follows. You must provide values for each
argument, even if some are unused:

```shell
./<program>_simplified <arg1> <arg2> <arg3> <arg4>
```

#### Optimized instantiator with Python

This version is still experimental. It does not support symbolic
loop bounds or symbolic WCETs, only symbolic procedure arguments.

First, transpile the simplified formula into a python object:

```shell
<wsymb>/simplify/swymplify -o <program>_simplified.py -c -p <program.pwf>
```

The generated file contains references that need to be updated with
respect to your installation of WSymb:

```shell
sed -i 's/..\/code_generator/<wsymb>\/compiler/g' <program>_simplified.py
sed -i 's/import generate/import compiler/g' <program>_simplified.py
```

Generate the optimized C instantiator:

```shell
python3 <program>_simplified.py > <program>_simplified.c
```

This method generates a C code that eases optimization by GCC. Compile
it as follows:

```shell
arm-linux-gnueabi-gcc -O3 -fno-stack-protector -static -o <program>_simplified.elf <wsymb>/compiler.test.c <program>_simplified.c
```
An important difference compared to the other executable files is that we must
search into the generated C code which procedure arguments
must be used to run the program (for optimization reasons).
Then, we can run the instantiator program and provide the
correct arguments:

```shell
./<program>_simplified.elf arg? ...
```

----

## References

[1] Sandro Grebant, Clément Ballabriga, Julien Forget, Giuseppe
Lipari. WCET analysis with procedure arguments as parameters. RTNS
2023: The 31st International Conference on Real-Time Networks and
Systems, Jun 2023, Dortmund, Germany. pp.11-22,
https://hal.science/hal-04118213.

