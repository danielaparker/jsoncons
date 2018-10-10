- Add `semantic_tag_type::decimal`. Map cbor Decimal Fractions and 
  Bigfloats to string values with this tag.

- Improve `cbor_view`

- Support pull parsing for cbor and CSV.

- Support selecting CSV fields using `json_pointer`

- Revist `encode_json` and `decode_json` now that we have pull parsing.

- Support more error recovery and introduce optional `lenient_error_handler`.

- Improve MessagePack support

- Support more binary data formats (BSON)

- Support [JSON Content Rules](https://datatracker.ietf.org/doc/draft-newton-json-content-rules/) for schema validation in `jsoncons_ext`

