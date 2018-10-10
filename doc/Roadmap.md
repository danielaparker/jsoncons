- Add `semantic_tag_type::decimal`. Map cbor Decimal Fractions and 
  Bigfloats to string values with this tag.

- Revist `encode_json` and `decode_json` now that we have pull parsing.

- Support more error recovery and introduce optional `lenient_error_handler`.

- Support [JSON Content Rules](https://datatracker.ietf.org/doc/draft-newton-json-content-rules/) for schema validation in `jsoncons_ext`

