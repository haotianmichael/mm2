TARGET=mm2
include ../shared/Makefile.shared

$(TARGET): chain.o  alloc.o
	$(CC) chain.o alloc.o -lm $(SNIPER_LDFLAGS) -o $(TARGET)

chain.o:
	$(CC) -c archKernel/mm2_chain_dp.c -o chain.o

alloc.o:
	$(CC) -c  archKernel/kalloc.c -o alloc.o

run_$(TARGET): 
	../../run-sniper -v -n 1 -c gainestown -d result2 -- ./mm2

CLEAN_EXTRA=viz
