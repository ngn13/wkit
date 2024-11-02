package util

import (
	"fmt"
	"io"
	"reflect"
)

type Buffer struct {
	Data []byte
	Pos  int
}

func NewBuffer(_b ...interface{}) *Buffer {
	var (
		b   interface{} = nil
		buf []byte      = nil
	)

	if _b != nil && len(_b) > 0 {
		b = _b[0]

		switch reflect.TypeOf(b).Kind() {
		case reflect.String:
			buf = []byte(b.(string))

		case reflect.Slice:
			buf = b.([]byte)
		}
	}

	return &Buffer{
		Data: buf,
		Pos:  0,
	}
}

func (b *Buffer) String() string {
	return string(b.Data)
}

func (b *Buffer) Len() uint64 {
	return uint64(len(b.Data))
}

func (b *Buffer) Write(p []byte) (n int, err error) {
	b.Data = append(b.Data, p...)
	return len(p), nil
}

func (b *Buffer) Read(p []byte) (n int, err error) {
	if n = len(b.Data) - b.Pos; n < len(p) {
		err = io.EOF
	} else {
		n = len(p)
	}

	copy(p, b.Data[b.Pos:])
	return n, err
}

func (b *Buffer) Seek(offset int64, whence int) (int64, error) {
	switch whence {
	case io.SeekStart:
		b.Pos = 0 + int(offset)
	case io.SeekCurrent:
		return 0, fmt.Errorf("seek method not implemented")
	case io.SeekEnd:
		b.Pos = len(b.Data) + int(offset)
	default:
		return 0, fmt.Errorf("invalid whence: %d", whence)
	}

	if b.Pos < 0 {
		b.Pos = 0
		return 0, fmt.Errorf("negative position: %d", b.Pos)
	}

	return int64(b.Pos), nil
}
