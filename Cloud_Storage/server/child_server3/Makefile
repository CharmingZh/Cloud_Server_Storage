SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=poss
CC:=gcc
CFLAGS:=-g -Wall 
$(ELF):$(OBJS)
		gcc -o $@ $^ -pthread -lmysqlclient -ll8w8jwt -lmbedcrypto -lmbedx509
clean:
		rm -rf $(OBJS) $(ELF)

