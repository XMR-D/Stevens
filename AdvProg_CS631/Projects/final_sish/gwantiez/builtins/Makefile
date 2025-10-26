include ../generic_flags.mk

BUILTINS=

BUILTINS+=cd
BUILTINS+= echo


all:	
	@for builtin in ${BUILTINS}; do \
		make -C $$builtin; \
	done

clean:	
	@for builtin in ${BUILTINS}; do \
		make clean -C $$builtin; \
	done
