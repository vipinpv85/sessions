CC = gcc
CFLAGS =-I. -ggdb -O0 -g -Bdynamic
CFLAGS += -pthread -lnuma
SRC = myqueue.c main.c
OBJ = main.o myqueue.o


.PHONY: all

all: $(OBJ) 
	mkdir -p build
	$(CC) $(OBJ) -o build/test $(CFLAGS)
	rm -rf $(OBJ) 

$(OBJ): %.o: %.c
	@echo " input: $<"
	@echo " output: $@"
	$(CC) -c $< -fPIC $(CFLAGS)

#%.c:
#	echo "file - " $@
#	$(CC) $@.c $a.o $(CFLAGS)

clean:
	@echo " --- invoking clean --- "
	rm -rf build
