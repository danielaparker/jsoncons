# Roadmap

### For release 0.113

- Enhanced JSON serialization options.
  (Currently implemented on master.)

- Preserve CBOR semantic tagging of expected conversion of byte 
  strings to base64, base64url, or base16, and , unless overridden 
  in `json_serializing_options`, use in JSON serialization
  (implemented on master.)

- Preserve CBOR bigfloat tag when decoding/encoding
  (implemented on master.)

- Convert CBOR non text string keys to strings when decoding
  to json values (implemented on master.)

- Add options for floating point precision in 
  `csv_serializing_options`.

### For later releases

- Support pull parsing for cbor and CSV.

- Support selecting CSV fields using `json_pointer`

- Support more error recovery and introduce optional `lenient_error_handler`.

- Improve MessagePack support

- Remove names that have been deprecated for more than a year.

At this point we'll slap a Version 1.0.0 Full Release stamp on `jsoncons`
(we've been leading up to this since 2013.)

### Post 1.0.0

- Support cbor keys implemented using SIDs

- Support more binary data formats (BSON)

- Support [JSON Content Rules](https://datatracker.ietf.org/doc/draft-newton-json-content-rules/) for schema validation in `jsoncons_ext`

