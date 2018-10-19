## jsoncons data model

The jsoncons data model consists of the following major types:

- null
- bool
- int64
- uint64
- double
- string
- byte_string
- array
- object

It also allows optional semantic tagging of the major types:

 Type       | Valid semantic tags | Notes
------------|---------------------|---------------
 int64      | epoch_time          | Maps to CBOR epoch time (unsigned or negative integer)
 uint64     | epoch_time          | Maps to CBOR epoch time (unsigned integer)
 double     | epoch_time          | Maps to CBOR epoch time (double)
 string     | bignum              | Maps to JSON integers that overflow int64 and uint64, CBOR bignum 
            | decimal             | Maps to CBOR decimal fraction 
            | date_time           | Maps to CBOR date/time


