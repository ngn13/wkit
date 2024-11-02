SUBDIRS = user server

all:
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d; \
	done

format:
	for d in $(SUBDIRS); do \
		$(MAKE) format -C $$d; \
	done

.PHONY: format
