TARGET=mm2
GEM5_ROOT=/gem5/gem5
CFLAGS = -g -O0
LDFLAGS = -static

$(TARGET): libm5.a
	$(CC) archKernel/mm2_chain_dp.c archKernel/kalloc.c -o mm2 -I$(GEM5_ROOT)/include -DGEM5 -L. -lm5

clean:
	rm mm2

