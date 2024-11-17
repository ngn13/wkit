VERSION = $(shell grep 'const VERSION' server/config/config.go | cut -d' ' -f4 | sed 's/"//g')
SUBDIRS = user kernel server

format:
	for d in $(SUBDIRS); do \
		$(MAKE) format -C $$d; \
	done

USER_SRC   = $(shell find user/ -type f -name '*.c' -or -name '*.h' -or -name 'Makefile')
KERNEL_SRC = $(shell find kernel/ -type f -name '*.c' -or -name '*.h' -or -name 'Makefile')

release:
	mkdir -pv release
	tar czf "release/shrk-client-$(VERSION).tar.gz" $(USER_SRC) $(KERNEL_SRC)

.PHONY: format release
