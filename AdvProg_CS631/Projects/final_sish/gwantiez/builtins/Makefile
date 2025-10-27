include ../generic_flags.mk

BUILTINS=

BUILTINS+=cd
BUILTINS+= echo


all:	
	@i=0; \
	 for builtin in ${BUILTINS}; do \
		i=$$((i+1)); \
		echo [$$i/${NB_BUILTINS}]: Compiling $$builtin; \
		make -C $$builtin; \
		echo ""; \
	done

clean:	
	@for builtin in ${BUILTINS}; do \
		make clean -C $$builtin; \
	done
