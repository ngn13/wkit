package log

import (
	"log"
	"os"
)

var (
	Info = log.New(os.Stdout, "\033[34m[info]\033[0m ", log.Ltime|log.Lshortfile).Printf
	Warn = log.New(os.Stderr, "\033[33m[warn]\033[0m ", log.Ltime|log.Lshortfile).Printf
	Fail = log.New(os.Stderr, "\033[31m[fail]\033[0m ", log.Ltime|log.Lshortfile).Printf
	Debg = log.New(os.Stderr, "\033[36m[debg]\033[0m ", log.Ltime|log.Lshortfile).Printf
)
