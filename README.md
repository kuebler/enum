Installation
============

1. get [picosat][1], follow its installation instructions 
2. modify the Makefile (adjust LIBS and INCLUDE to your needs)
3. issue `make`

Usage
=====

Works on DIMACS files whose first comment line contains the number of free variables *f*. Enum expects that variables 1..*f* are free, all the over variables are considered bound. Example:

    c 3
    p cnf 5 1
    1 2 3 4 5 0

is used for &exist; x4, x5. x1 &or; x2 &or; x3 &or; x4 &or; x5. The QE approach is inspired by [Brauer, King, *Kriener: Existential Quantification as Incremental SAT*, CAV 2011][2].

[1] http://fmv.jku.at/picosat/
[2] http://embedded.rwth-aachen.de/lib/exe/fetch.php?media=bib:bkk11a.pdf
