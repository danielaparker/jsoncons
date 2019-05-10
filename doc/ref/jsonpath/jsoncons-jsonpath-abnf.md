```
path = absolute-path / relative-path

absolute-path = root *additional-element

root = "$"

relative-path = sp (name / wildcard) sp *additional-element

sp = *( WSP / CR / LF )

additional-element = ("." sp name sp) / ("." wildcard) / predicate

predicate = "[" expr "]"

name = unquoted-name / single-quoted-name / double-quoted-name

expr = wildcard / relative-path / slice / union / filter

wildcard = "*"

name = unquoted-name / single-quoted-name / double-quoted-name

unquoted-name = unquoted-name-character *additional-unquoted-name-character

single-quoted-name = "'" *single_quoted-name-character "'"

double-quoted-name = '"' *double_quoted-name-character  '"'

unquoted-name-character = ? any unicode character except *, spaces, '.' and '[' ?

additional-unquoted-name-character = ? any unicode character except spaces, '.' and '[' ?

single-quoted-name-character = ? any unicode character except an unescaped "'" (single quote) ?

double-quoted-name-character = ? any unicode character except an unescaped '"' (double quote) ?

slice = [ signed-integer ] ":" [ signed-integer ] [ ":" [ non-zero-signed-integer ] ]

filter = "?(" filter-expr ")"

union = relative-path "," relative-path *("," relative-path)
```

