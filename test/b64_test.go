package xxx

import (
	"encoding/base64"
	"testing"
	"os/exec"
)

var (
	enc = base64.NewEncoding("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
)

func FuzzEncode(f *testing.F){
	f.Add("lorem ipsum")
	f.Fuzz(func(t *testing.T, src string) {
		out, err := exec.Command("./a.out", "encode", src).Output()
		if err != nil {
			t.Fatalf("failed to run 'b64 encode '%v'", src)
		}
		out = out[:len(out)-1] // drop the last char, because it's a newline added by b64.c
		expected := enc.EncodeToString([]byte(src))
		actual := string(out)
		if actual != expected {
			t.Logf("Encoding '%v': expected '%v', but got '%v'\n", src, expected, actual)
			t.Fail()
		}
	})
}

func FuzzDecode(f *testing.F){
	f.Add(enc.EncodeToString([]byte("lorem ipsum")))
	f.Fuzz(func(t *testing.T, src string) {
		out, err := exec.Command("./a.out", "decode", src).Output()
		if err != nil {
			t.Fatalf("failed to run 'b64 decode '%v'", src)
		}
		out = out[:len(out)-1] // drop the last char, because it's a newline added by b64.c
		expectedB, err := enc.DecodeString(src)
		if err != nil {
			t.Fatalf("Failed to decode '%v'", src)
		}
		expected := string(expectedB)
		actual := string(out)
		if actual != expected {
			t.Logf("Encoding '%v': expected '%v', but got '%v'\n", src, expected, actual)
			t.Fail()
		}
	})
}
