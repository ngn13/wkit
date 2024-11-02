GOSRCS = $(shell find . -type f -name '*.go')

all: shrk-server.elf

shrk-server.elf: $(GOSRCS)
	go build -o $@

format:
	gofmt -s -w .

debug:
	SHRK_DEBUG=1 SHRK_PATH=/test SHRK_PASSWORD=test ./shrk-server.elf

.PHONY: format
