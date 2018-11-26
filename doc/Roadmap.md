# Roadmap

### For release 0.114.0

MessagePack 

- New msgpack_serializer that supports Message Pack bin formats 
  (now on master)

- New msgpack_parser that supports Message Pack bin formats 
  (now on master)

- Add semantic tag types to accomodate Message Pack timestamp 32,
  64 and 96, and support conversion with CBOR epoch time.

- Improve error code for cbor and msgpack parsing.

### For later releases

- Support pull parsing for cbor, msgpack, and CSV.

- Support more error recovery and introduce optional `lenient_error_handler`.

- Remove names that have been deprecated for more than a year.

- Support more binary data formats (BSON, UBJSON)

At this point we'll slap a Version 1.0.0 Full Release stamp on `jsoncons`
(we've been leading up to this since 2013.)

### Post 1.0.0

- Support cbor keys implemented using SIDs

- Support [JSON Content Rules](https://datatracker.ietf.org/doc/draft-newton-json-content-rules/) for schema validation in `jsoncons_ext`

