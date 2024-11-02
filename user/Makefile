# sources
CSRCS = $(shell find ./ -type f -name '*.c')
HSRCS = $(shell find ./inc/ -type f -name '*.h')

# config
SHRK_DEBUG       = 1
SHRK_SERVER_ADDR = 127.0.0.1
SHRK_SERVER_PORT = 1053
SHRK_CLIENT_ID   = DEBUGID0
SHRK_CLIENT_KEY  = DEBUGINGKEY0

# compiler args
VARS    = -DSHRK_DEBUG=$(SHRK_DEBUG)                 \
					-DSHRK_SERVER_ADDR=\"$(SHRK_SERVER_ADDR)\" \
					-DSHRK_SERVER_PORT=$(SHRK_SERVER_PORT)     \
					-DSHRK_CLIENT_ID=\"$(SHRK_CLIENT_ID)\"     \
					-DSHRK_CLIENT_KEY=\"$(SHRK_CLIENT_KEY)\"
CFLAGS  = -O3
INCLUDE =

all: shrk-user.elf

shrk-user.elf: $(CSRCS) $(HSRCS)
	gcc $(CFLAGS) $(INCLUDE) $(VARS) $(CSRCS) -o $@

format:
	clang-format -i -style=file $(CSRCS) $(HSRCS)

.PHONY: format
