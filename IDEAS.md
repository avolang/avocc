# Draft and ideas

This document keeps a draft of the ideas to the compiler and language specification.

## Compiler stages

The compiler pipeline should transform code into CPS form and then perform optimizations,
this is the general idea for the first steps:

- STAGE0 - `MOD_TABLE`, Module connections (not gonna do it yet)
  - Find module name
  - Find imports
  - Find exports
  - Find externs
- STAGE1 - `FIX_TABLE`, Collect mutually-recursive objs (this is what I'm currently focusing on)
  - Find defs and add a root fix, create scopes if needed
  - Find lambdas and add to scope fix table, create scopes if needed
- STAGE2 - `BODIES_TO_CPS`, Transform to CPS all bodies of fix tables, this will create a scope for each continuation.
- STAGE3 - `MAKE_CLOSURES`, Transform all nested fix tables into a unique root fix table.
