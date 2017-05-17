# Last Update:2017-05-16 10:28:23
#########################################################################
# File Name: Makefile
# Author: wangchenxi
# mail: chinawangchenxi@gmail.com
# Created Time: 2017年05月16日 星期二 10时28分23秒
# brief:
#########################################################################
CC:=g++
CFLAGS:= -Wall -O2 -std=c++11
INCLUDEFLAGS:=
BIN:=test
LDFLAGS:=
OBJ:=test_vector.o

.PHONY:all
all:$(BIN)
$(BIN):$(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)

-include $(OBJ:.o=.d)

%.o:%.cpp
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDEFLAGS)
%.d:%.cpp
	@set -e;$(CC) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
	sed 's,\($*\).o[:],\1.o $@:,g' < $@.$$$$ >$@; \
	rm -f $@.$$$$
.PHONY: clean

clean:
	rm -rf *.o $(BIN) *.d *.d.*

