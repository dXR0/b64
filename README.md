## b64

This is a cli tool to encode/decode to/from base64.

## Getting started

```sh
$ gcc -o b64 b64.c
$ ./b64 e "lorem ipsum" | ./b64 d
lorem ipsum
```

## Synopsis

```sh
b64 ACTION [ARGUMENT]...
```

## Description

There are 2 ACTIONS: `e` to encode and `d` to decode.

There are 4 ways to use `b64`:

* pass ARGUMENTs to `b64`
```sh
$ ./b64 e "lorem ipsum" test "test2"
bG9yZW0gaXBzdW0=
dGVzdA==
dGVzdDI=

$ ./b64 d bG9yZW0gaXBzdW0= dGVzdA== dGVzdDI=
lorem ipsum
test
test2
```
* read stdin
```sh
$ printf "lorem ipsum" | ./b64 e
bG9yZW0gaXBzdW0=

$ printf "bG9yZW0gaXBzdW0=" | ./b64 d
lorem ipsum
```
* read file, that is piped as stdin
```sh
$ printf "lorem ipsum" > file1
$ ./b64 e < file1 > file2
$ cat file2
bG9yZW0gaXBzdW0=

$ ./b64 d < file2
lorem ipsum
```
* start a REPL, where each input line is encoded(/decoded)
```sh
$ ./b64 e
lorem ipsum
bG9yZW0gaXBzdW0=

$ ./b64 d
bG9yZW0gaXBzdW0=
lorem ipsum
```

NOTE: If any ARGUMENT(s) are provided, then stdin/file piping is ignored.

NOTE: For decoding, only one value at a time can be decoded. For encoding, if multiple line separated values are provided, then it is considered as one value and encoded as such.
When providing multiple values to be decoded as ARGUMENTs, then all provided arguments will be decoded.

## Author

Meelis Utt
