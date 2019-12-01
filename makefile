DIRS:=$(shell find . -mindepth 2 -maxdepth 2 -name "makefile")
DIRS:=$(subst /makefile,'',$(DIRS))
all:
	for i in ${DIRS}; \
	do  \
		make -C $$i; \
	done
	
.PHONY:clean
clean:
	for i in ${DIRS}; \
	do \
		make -C $$i clean; \
	done
