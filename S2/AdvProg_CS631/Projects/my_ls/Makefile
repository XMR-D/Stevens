include mkflags/generic_flags.mk

#list all sources files in src/ dir, using a wildcard to prevent dev from adding every source file by hand.
srcs-y:= $(wildcard src/*.c src/*.S)

# Create all objects files from C and Asm source files
objs-y:=$(patsubst %.c,%.o,$(srcs-y))
objs-y:=$(patsubst %.S,%.S.o,$(objs-y))


#specify that "all" is not a file and rather a command.
.PHONY: all
all: ls

ls: $(objs-y)
	$(CC) $(CFLAGS) -I$(INCLUDE) $^ -o $@

debug :
	make debug_flag 'CFLAGS=$(DEBUGFLAGS)'
debug_flag: $(objs-y)
	$(CC) $(CFLAGS) -I$(INCLUDE) $^ -o debug_ls

#C and Asm source file compilation with Classic flags
%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $^ -o $@

%.S.o: %.S
	$(AS) $(SFLAGS) -I$(INCLUDE) -c $^ -o $@

#specify that "clean" is not a file and rather a command.
.PHONY: clean
clean:
	rm -rdf ls debug_ls $(SRC_DIR)/*.o