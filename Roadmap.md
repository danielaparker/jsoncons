# Roadmap

### For later releases

- Support CBOR tags `256` (mark value as having string references) and `25` (reference the nth previously seen string) [http://cbor.schmorp.de/stringref]
  (on master)

At this point we'll slap a Version 1.0.0 Full Release stamp on `jsoncons`
(we've been leading up to this since 2013.)

### Post 1.0.0

- Support more error recovery and introduce optional `lenient_error_handler`.

- Support JSON Schema https://json-schema.org/ for schema validation in `jsoncons_ext`

- Support [JSON Content Rules](https://datatracker.ietf.org/doc/draft-newton-json-content-rules/) for schema validation in `jsoncons_ext`

