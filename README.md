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
EOF := /* End of file */
EOL := /* New line character */
UNICODE_ID := /* All characters except terminators space */
UNICODE_NO_NEWLINE := /* All characters except new line */
UNICODE := /* All unicode characters */
LIST_START := '['
LIST_END := ']'
CALL_START := '<' | '('
CALL_END := '>' | ')'

NIL := 'nil'
LIT_INT := { '0x' | '0b' | '0o' } [ DIGIT ] { 'i32' | 'i64' }
LIT_FLO := { '-' } [ DIGIT ] { '.' } [ DIGIT ] { 'f32' | 'f64' }
LIT_BOL := true | false
LIT_STR := ( ''' | '"' ) [ UNICODE_NO_NEWLINE ] ( ''' | '"' ) | '`' [ UNICODE ] '`'
LIT := LIT_INT | LIT_FLO | LIT_BOL | LIT_STR

LINE_COMMENT := ';' [ UNICODE_NO_NEWLINE ] EOL
BLOCK_COMMENT := ';;' [ UNICODE ] ';;'
COM := LINE_COMMENT | BLOCK_COMMENT

TYP := COLON [ UNICODE_ID ] | COLON LIST
SYM := [ UNICODE_ID ] | [ UNICODE_ID ] TYP

ITEM := COM | LIT | SYM | NIL | LIST
LIST := LIST_START [ ITEM ] LIST_END
SOURCE := [ LIST ] EOF
```
