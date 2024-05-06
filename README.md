# Kofola: modular complementation and inclusion checking for omega automata

Kofola has been built on top of [SPOT](https://spot.lrde.epita.fr/) and
inspired by [Seminator](https://github.com/mklokocka/seminator) and
[COLA](https://github.com/liyong31/COLA).


### Requirements
* [Spot](https://spot.lrde.epita.fr/)

```
./configure --enable-max-accsets=128
```
One can set the maximal number of colors for an automaton when configuring Spot with --enable-max-accsets=INT
```
make && make install
```

### Compilation
Please run the following steps to compile Kofola after cloning this repo:
```
mkdir build && cd build
cmake ..
make
```

Then you will get an executable in `build/src/kofola`.

To use `kofola` for HyperLTL model checking, an example of execution is as follows (assuming we are in `build` directory):
```
./src/kofola --hyperltl_mc ../hyperltl_ex/bp/concur_p1_1bit.hoa ../hyperltl_ex/bp/gni.hq
```

To use `kofola` for inclusion checking, an example of execution is as follows (assuming we are in `build` directory):
```
./src/kofola --inclusion ../inclusion_ex/bakery_3procs_bakery_formula_S2_3proc_A.hoa ../inclusion_ex/bakery_3procs_bakery_formula_S2_3proc_B.hoa
```

### HyperLTL MC input formats
#### System
As input format for system behavior, we decided to use the HOA format so that it can
be easily parsed and stored by Spot as a Kripke structure. An example:
```
HOA: v1
States: 4
Start: 3
AP: 3 "h_0" "l_0" "o_0" 
acc-name: all
Acceptance: 0 t
properties: state-labels explicit-labels state-acc deterministic
--BODY--
State: [!0&!1&!2] 2
3 
State: [!0&!1&!2] 3
4 
State: [2&!0&!1] 4
5 
State: [2&!0&!1] 5
2 
--END--
```
#### Formula
For LTL body of the HyperLTL formula we support the exact format that `Spot` supports. However, each atomic proposition (AP) 
is of the format `{ap_sys}_{trace_var}` with `ap_sys` standing for the atomic proposition used within the system and 
`trace_var` stands for the quantified trace. The `f` formula with quantifiers is then generated by the following syntax:
```
  f ::= (forall trace_var. exists trace_var.)* LTL 
  trace_var ::= string 
```
An example of a property for the above system:
```
forall A. forall B. exists C. (G ("{h_0}_{A}" <-> "{h_0}_{C}")) & (G("{l_0}_{B}" <-> "{l_0}_{C}"))
```