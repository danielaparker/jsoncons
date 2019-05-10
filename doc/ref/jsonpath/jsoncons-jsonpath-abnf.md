```
Path = AbsolutePath / RelativePath

AbsolutePath = Root *AdditionalElements

Root = "$"

RelativePath = S NameOrWildcard S *AdditionalElements

NameOrWildcard = Name / Wildcard

S = *( WSP / CR / LF )

AdditionalElements = ("." / "..") NameOrWildcard / Predicate

Predicate = "[" Expr "]"

Name = UnquotedName / SingleQuotedName / DoubleQuotedName

Expr = Wildcard / RelativePath / Slice / Union / Filter

Wildcard = "*"

Name = UnquotedName / SingleQuotedName / DoubleQuotedName

UnquotedName = UnquotedNameCharacter *AdditionalUnquotedNameCharacter

SingleQuotedName = "'" *SingleQuotedNameCharacter "'"

DoubleQuotedName = '"' *DoubleQuotedNameCharacter  '"'

UnquotedNameCharacter = ? any unicode character except *, spaces, '.' and '[' ?

AdditionalUnquotedNameCharacter = ? any unicode character except spaces, '.' and '[' ?

SingleQuotedNameCharacter = ? any unicode character except an unescaped "'" (single quote) ?

DoubleQuotedNameCharacter = ? any unicode character except an unescaped '"' (double quote) ?

Slice = [ SignedInteger ] ":" [ SignedInteger ] [ ":" [ NonZeroSignedInteger ] ]

Filter = "?(" Filter-Expr ")"

Union = RelativePath "," RelativePath *("," RelativePath)
```

