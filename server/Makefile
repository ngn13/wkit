GOSRCS = $(shell find . -type f -name '*.go')

all: shrk_server.elf

shrk_server.elf: $(GOSRCS)
	go build -o $@

format:
	gofmt -s -w .

clean:
	rm *.elf

debug:
	SHRK_DEBUG=1 SHRK_PATH=/test SHRK_PASSWORD=test ./shrk_server.elf

.PHONY: format clean debug
