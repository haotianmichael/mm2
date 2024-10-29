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

` ./minimap2 -a test/MT-human.fa  test/MT-orang.fa --chain-dump-in in1.txt  --chain-dump-out out-1k.txt  --chain-dump-limit=1000  > test.sam
  
` ./minimap2 -ax map-ont -t 16 hg38.fa  na12878.fastq --chain-dump-in in.txt --chain-dump-limit=1 > test.sam  

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


## output format
### in1.txt
346(number of anchors each read) 15.000000(average length of one anchor, 15 by default) 5000(max_dist_x)  5000(max_dist_y)  500(bandwidth)
0() 40(anchor position in reference)  15  16065(anchor position in query)

### out-1.txt
346(number of anchors each read)
f(score) p(predecessor) v(not impotant) t(not impotant)
3 15  -1  15  3


