SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,$(DIST)/%.o,$(SRCS))
DIST = dist

all: $(DIST) $(OBJS)

$(DIST):
	mkdir -pv $(DIST)

$(DIST)/%.o: %.c common.h
	$(CC) $< -o $@

format:
	clang-format -i -style=file $(SRCS) *.h

clean:
	rm -r $(DIST)

.PHONY: format clean
