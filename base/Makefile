TARGET=mm2
# for sniper simulation
#include ../shared/Makefile.shared

$(TARGET): chain.o  alloc.o
	$(CC) -g -O0 chain.o alloc.o -o $(TARGET)
#	$(CC) chain.o alloc.o -lm $(SNIPER_LDFLAGS) -o $(TARGET)

chain.o:
	$(CC) -c -g -O0 archKernel/mm2_chain_dp.c -o chain.o

alloc.o:
	$(CC) -c -g  -O0 archKernel/kalloc.c -o alloc.o

#run_$(TARGET): ../../run-sniper -v -n 1 -c gainestown -d result2 -- ./mm2
clean:
	rm *.o mm2

