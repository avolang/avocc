# Avocado

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
DIGIT := /* 0-9 */
NEW_LINE := /* New line character */
UNICODE_SYMBOL := /* All characters except <{([])}> and space*/
UNICODE := /* All unicode characters */
UNICODE_NO_NEWLINE := /* All unicode characters except new line */
LIST_START := /* <{[( */
LIST_END := /* )]}> */

NIL := 'nil'
INT_LITERAL := { '0x' | '0b' | '0o' } [ DIGIT ] { 'i32' | 'i64' }
FLO_LITERAL := { '-' } [ DIGIT ] { '.' } [ DIGIT ] { 'f32' | 'f64' }
BOL_LITERAL := true | false
STR_LITERAL := ( ''' | '"' ) [ UNICODE_NO_NEWLINE ] ( ''' | '"' ) | '`' [ UNICODE ] '`'
LITERAL := INT_LITERAL | FLO_LITERAL | BOL_LITERAL | STR_LITERAL

LINE_COMMENT := ';' [ UNICODE ] NEW_LINE
BLOCK_COMMENT := ';;' [ UNICODE ] ';;'
COMMENT := LINE_COMMENT | BLOCK_COMMENT

TYPE_SPEC := COLON [ UNICODE_SYMBOL ] | COLON LIST
SYMBOL := [ UNICODE_SYMBOL ] | [ UNICODE_SYMBOL ] TYPE_SPEC

ATOM := SYMBOL | LITERAL | NIL
ITEM := ATOM | LIST | COMMENT
LIST := LIST_START [ ITEM ] LIST_END
SOURCE := [ LIST ]
```
