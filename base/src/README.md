## version: minimap2-2.17

## simple example:
```
cd src
make 
./minimap2 -a test/MT-human.fa  test/MT-orang.fa > test.sam

```

## dump anchors and scores
* --chain-dump-limit: the number of reads you want to dump
* --chain-dump-in: dump in.txt which includes all anchors of specific number of reads.
* --chain-dump-out: dump out.txt which includes score and predecessor of each anchor

```

./minimap2 -a test/MT-human.fa  test/MT-orang.fa --chain-dump-in in1.txt  --chain-dump-out out-1k.txt  --chain-dump-limit=1000  > test.sam

# There are two files generated:
#   in-1k.txt: the input of the chaining function
#     for 1,000 reads.
#   out-1k.txt: the output of the corresponding
#     chaining tasks.
# You can use them to run benchmarks of different
#   kernels, and compare results to ensure
#   correctness.
```


## modified file
> chain.c main.c

