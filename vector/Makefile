# Last Update:2017-05-16 10:28:23
#########################################################################
# File Name: Makefile
# Author: wangchenxi
# mail: chinawangchenxi@gmail.com
# Created Time: 2017年05月16日 星期二 10时28分23秒
# brief:
#########################################################################
CC:=g++
test: test_vector.o test_vector.h test_vector.d
	$(CC) $^ -o $@
%.d:%.c
	$(CC) -MM $< >$@.$$$$; \
	sed 's,\($*\).o[:]*,\1.o $@:,g' < $@.$$$$ >$@; \
	rm -f $@.$$$$
.PHONY: clean

clean:
	rm -rf *.o

