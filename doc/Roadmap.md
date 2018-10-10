# Roadmap

### Targeting release 0.111.0

- Add `semantic_tag_type::decimal`. Map cbor Decimal Fractions and 
  Bigfloats to string values with this tag.

- Improve `cbor_view`

- Document jsoncons data model.

- Review names with eye to consistency with C++ standard names.
  `has_key()` will be deprecated and renamed to `contains()`.

### For later releases

- Support pull parsing for cbor and CSV.

- Support selecting CSV fields using `json_pointer`

- Revist `encode_json` and `decode_json` now that we have pull parsing.

- Support more error recovery and introduce optional `lenient_error_handler`.

- Improve MessagePack support

- Revisit JSON serialization options

- Remove names that have been deprecated for more than a year.

At this point we'll slap a Version 1.0.0 Full Release stamp on `jsoncons`
(we've been leading up to this since 2013.)

### Post 1.0.0

- Support cbor keys implemented using SIDs

- Support more binary data formats (BSON)

- Support [JSON Content Rules](https://datatracker.ietf.org/doc/draft-newton-json-content-rules/) for schema validation in `jsoncons_ext`

