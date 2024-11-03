package log

import (
	"io"
	"log"
	"os"
)

const (
	FILE = "/tmp/_shrk_server.log"

	COLOR_BLUE   = "\033[34m"
	COLOR_YELLOW = "\033[33m"
	COLOR_RED    = "\033[31m"
	COLOR_CYAN   = "\033[36m"
	COLOR_RESET  = "\033[0m"
)

var (
	Info func(format string, v ...any)
	Warn func(format string, v ...any)
	Fail func(format string, v ...any)
	Debg func(format string, v ...any)
)

func New() error {
	var (
		temp_log *os.File
		err      error
	)

	if temp_log, err = os.Create(FILE); err != nil {
		return err
	}

	stdout := io.MultiWriter(os.Stdout, temp_log)
	stderr := io.MultiWriter(os.Stderr, temp_log)

	Info = log.New(stdout, COLOR_BLUE+"[info]"+COLOR_RESET+" ", log.Ltime|log.Lshortfile).Printf
	Warn = log.New(stderr, COLOR_YELLOW+"[warn]"+COLOR_RESET+" ", log.Ltime|log.Lshortfile).Printf
	Fail = log.New(stderr, COLOR_RED+"[fail]"+COLOR_RESET+" ", log.Ltime|log.Lshortfile).Printf
	Debg = log.New(stderr, COLOR_CYAN+"[debg]"+COLOR_RESET+" ", log.Ltime|log.Lshortfile).Printf

	return nil
}
