# C-style C++ conventions (NASA/JPL-inspired)

This project retains **C++17**, STL containers and exceptions-free control flow in
its core routines. It is NOT certified NASA/JPL-compliant or flight software.

Guidelines:
- Explicit return codes and error strings for file parsing; check every result.
- Braces on every control-flow block, including single-statement blocks.
- Simple functions, local variables and predictable iteration bounds.
- Fixed-width integer types for on-disk values where relevant.
- No raw owning pointers or unchecked index arithmetic.
- Avoid recursion, macros for logic, global mutable state and hidden side effects.
- Compile with GCC/Clang warnings enabled and run regression tests.
- C-like names and syntax, while using C++ standard containers for memory safety.

References:
- NASA Software Engineering Handbook, Coding Standards:
  https://swehb.nasa.gov/spaces/SWEHBVC/pages/84279577/9.03%2BCoding%2BStandards
- Gerard J. Holzmann, The Power of Ten (NASA/JPL):
  https://www.state-machine.com/doc/NASA-JPL06c.pdf
