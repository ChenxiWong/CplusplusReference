# Last Update:2017-05-27 09:07:25
#########################################################################
# File Name: Makefile
# Author: wangchenxi
# mail: chinawangchenxi@gmail.com
# Created Time: 2017年05月27日 星期六 09时07分25秒
# brief:
#########################################################################
.PHONY:clean all
CC=gcc
CFLAGS=  -Wall -g
LDFLAGS= -lpthread
DFLAGS=
BIN=thread_test
all:$(BIN)
%.o:%.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) $(DFLAGS)  -o $@
.PHONY: clean
clean:
	rm -rf $(BIN) *.o
