DEPS=dberror.h storage_mgr.h buffer_mgr_stat.h buffer_mgr.h dt.h test_helper.h
O1=dberror.o storage_mgr.o
O2=dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o

%.o: %.c $(DEPS)
	gcc -c -o $@ $<

all: test_assign2_1 test_assign2_2

dberror.o: dberror.c
	gcc -c dberror.c

storage_mgr.o: storage_mgr.c
	gcc -c storage_mgr.c

test_assign2_1: $(O2)	test_assign2_1.o
	gcc -o $@ $^ -lpthread

buffer_mgr_stat.o: buffer_mgr_stat.c
	gcc -c buffer_mgr_stat.c

buffer_mgr.o: buffer_mgr.c
	gcc -c buffer_mgr.c

test_assign2_1.o: test_assign2_1.c
	gcc -c test_assign2_1.c

test_assign2_2: $(O2)	test_assign2_2.o
	gcc -o $@ $^ -lpthread

test_assign2_2.o: test_assign2_2.c
	gcc -c test_assign2_2.c

.PHONY: clean
clean:
	-rm -f test_assign2_1 test_assign2_2 *.o *.log

run:
	./test_assign2_1
	./test_assign2_2

