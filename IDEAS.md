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

## EBNF

This EBNF syntax productions will be used to generate the tables for a possible LR parser

```
LIT -> STR
LIT -> BOL
LIT -> LST

ISQ -> ISQ ITM
ISQ -> ESQ ISQ
ISQ -> ITM
ISQ -> KWI
LST -> [ ISQ ]
LST -> []

ITM -> SYM
ITM -> LIT
ITM -> CAL
ITM -> BLO
CAL -> ( ITM )
CAL -> ( ITM ISQ )
KWI -> ITM : ITM

LIN -> ISQ EOL
LIN -> EOF
LIN -> EOL
LSQ -> LSQ LIN
LSQ -> LIN
BLO -> { LSQ }

S -> LSQ
S' -> S
```

One think that might change it's the way symbols are handled, I'm thinking
to remove the name-type pairs in favor of "keyword-items" which may simulate
the same behaviour after some custom transformations after parsing.

Also, kw-items will enable so much possibilities, consider the following snippet:

```
(log message:"hello ${name}!" name:"avocado")
```

Or, even more interesting:

```
def (some-op left|a:i32 right|b:i32):i32 (+ (mul a 2) (mul b 3))
some-op left:1 right:2
```

Both examples can be expanded using matching and processing right before CPS transformations.
