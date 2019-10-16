# Avocado
[![CircleCI](https://circleci.com/gh/avolang/avocc/tree/master.svg?style=svg)](https://circleci.com/gh/avolang/avocc/tree/master)

Avocado is a general propouse, functional, static and strong typed
programming language inspired and based on LISP family. Avocado is
currently a toy, as I work on it I might convert it into a more formal
solution, but I don't plan anything like that yet in the near future.

## Avocado C compiler

This repository holds the C compiler and the first specification
of the language.

## Version 0.0.1

This is the first specification of avocado, the unicode support
is not totally planned for the first implementation, however
some support may be available.

### Lex of avocado

```
COLON := ':'
DIGIT := /* 0-9 */
EOL := /* New line character */
LLS := '<' | '(' | '['
LLE := '>' | ')' | ']'
VLS := '{'
VLE := '}'
NIL := 'nil'
UNICODE := /* All unicode characters */
UNICODE_NO_SPACE := /* All characters except space or new line */
UNICODE_NO_NEWLINE := /* All unicode characters except new line */

BOL_LIT := true | false
INT_LIT := { '0x' | '0b' | '0o' } [ DIGIT ] { 'i32' | 'i64' }
FLO_LIT := { '-' } [ DIGIT ] { '.' } [ DIGIT ] { 'f32' | 'f64' }
STR_LIT := '"' [ UNICODE_NO_NEWLINE ] '"'
LITERAL := INT_LIT | FLO_LIT | BOL_LIT | STR_LIT

LINE_COMMENT := ';' [ UNICODE ] EOL
BLOCK_COMMENT := ';;' [ UNICODE ] ';;'
COMMENT := LINE_COMMENT | BLOCK_COMMENT

TYP := COLON [ UNICODE_NO_SPACE ] | COLON HLST
SYM := [ UNICODE_NO_SPACE ] | [ UNICODE_NO_SPACE ] TYP

ITEM := COMMENT | LITERAL | SYMBOL | NIL | HLST | VLST
HLST := LLS [ ITEM ] LLE
LLST := [ ITEM ] EOL
VLST := VLS [ LLST ] VLE
SOURCE := [ LLST ] EOF
```
