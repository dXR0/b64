#!/bin/sh
cc ../b64.c
rm -rf testdata*
rm -f xxx.test
go test -c b64_test.go
./xxx.test -test.fuzzcachedir=testdata_encode -test.fuzz=Encode -test.fuzztime=10s
./xxx.test -test.fuzzcachedir=testdata_decode -test.fuzz=Decode -test.fuzztime=10s
rm -f xxx.test a.out
