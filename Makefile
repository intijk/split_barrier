CC=oshcc
all: split_inc block_inc
split_inc: split_inc.c
	$(CC) split_inc.c -o split_inc -D HAVE_FEATURE_SPLIT_BARRIER
block_inc: block_inc.c
	$(CC) block_inc.c -o block_inc
clean:
	rm split_inc block_inc
run:
	oshrun -np 16 ./split_inc 64
runblock:
	oshrun -np 16 ./block_inc 64
alloc:
	salloc -w shark25,shark26
