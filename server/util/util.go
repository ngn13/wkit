package util

import (
	"encoding/hex"
	"fmt"
	"math/rand"
	"os"
	"path"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/config"
)

func ListNext(l []string, i uint64) string {
	if i+1 >= uint64(len(l)) {
		return ""
	}
	return l[i+1]
}

func DirExists(path string) error {
	var (
		err error
		st  os.FileInfo
	)

	if st, err = os.Stat(path); err != nil {
		return err
	}

	if !st.IsDir() {
		return fmt.Errorf("specified path is not a directory")
	}

	return nil
}

const (
	chars       = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	chars_count = len(chars)
)

func RandomStr(n int) string {
	b := make([]byte, n)
	for i := range b {
		b[i] = chars[rand.Intn(chars_count)]
	}
	return string(b)
}

func Redirect(c *fiber.Ctx, p string) error {
	conf := c.Locals("config").(*config.Type)
	if p[0] == '/' {
		p = p[1:]
	}
	return c.Redirect(path.Join(conf.Path, p))
}

func StrNext(s *string, c int) string {
	if c < 0 {
		return *s
	}

	res := (*s)[:c]
	*s = (*s)[c:]
	return res
}

func DecodeHex(enc string) ([]byte, error) {
	var err error
	res := make([]byte, len(enc)/2)
	_, err = hex.Decode(res, []byte(enc))
	return res, err
}

func EncodeHex(dec []byte) (string, uint64) {
	size := len(dec) * 2
	res := make([]byte, size)
	hex.Encode(res, dec)
	return string(res), uint64(size)
}
