package util

import (
	"crypto/sha1"
	"encoding/hex"
	"fmt"
	"math/bits"
	"math/rand"
	"os"
	"path"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/config"
)

// https://red.ngn.tf/r/golang/comments/8micn7/review_bytes_to_human_readable_format/
func HumanizeSize(s uint64) string {
	if s < 1024 {
		return fmt.Sprintf("%d bytes", s)
	}

	base := uint(bits.Len64(s) / 10)
	val := float64(s) / float64(uint64(1<<(base*10)))

	return fmt.Sprintf("%.1f %ciB", val, " KMGTPE"[base])
}

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

func SHA1(s string) string {
	sum := sha1.Sum([]byte(s))
	return hex.EncodeToString(sum[:])
}
