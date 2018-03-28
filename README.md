# LLVM-Optimization

This is for study on UCSD CSE231 only. Any reference on this repository while taking CSE231 is a violation.

The project specification can be found in https://ucsd-pl.github.io/cse231/wi18/project.html. We use LLVM version 5.0.1 and focuse mainly on the target-independent backend that performs  various analyses and transformations on LLVM IR. There are three major analysis implemented under /Passes/DFA: 

1. Reaching Definition Analysis
2. Liveness Analysis
3. MayPointTo Analysis

To see how to generate LLVM IR from source file, how to compile and run LLVM pass, refer to /Guides/.
