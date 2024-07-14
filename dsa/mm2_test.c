#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "kalloc.h"

/*for allocation of memory*/
#define READ_NUM 3000
typedef int32_t loc_t;
typedef int32_t width_t;
typedef int32_t tag_t;
typedef struct
{
    uint64_t x, y;
} mm128_t;
typedef struct{
    tag_t tag;
    loc_t x;
    width_t w;
    loc_t y;
}anchor_t;
typedef struct{
    int64_t n;
    float avg_span;
    int max_dist_x, max_dist_y, bw;
    anchor_t* anchors;    
}read_t;
typedef struct{
    int rep_len, frag_gap;
    void *km;
}mm_tbuf_t;


void *kmalloc(void *km, size_t size);
void kfree(void *km, void *ptr);
void mm_chain_dp(int max_dist_x, int max_dist_y, int bw, int max_skip, int max_iter, int min_cnt, int min_sc, int is_cdna, int n_segs, int64_t n, mm128_t *a, int *n_u, uint64_t **_u, void *km, int32_t *f, int32_t *p, int32_t *t, int32_t *v);
void skip_to_EOR(FILE *fp) {
    const char *loc = "EOR";
    while(*loc != '\0') {
        if(fgetc(fp) == *loc) {
             loc++;
        }
    }
}

mm128_t *parse_read(void *km, FILE* fp,mm128_t* a, int64_t *n, int *max_dist_x, int *max_dist_y, int *bw) {

    float avg_qspan;
    int tt = fscanf(fp, "%lld%f%d%d%d", n, &avg_qspan, max_dist_x, max_dist_y, bw);

    
    //kmalloc read->anchors
    //read->anchors = (anchor_t*)kmalloc(km, read->n * sizeof(anchor_t)); 
    a = (mm128_t*)kmalloc(km, *n * sizeof(mm128_t));

    for(int i = 0; i < *n;i ++) {
        uint64_t tag;
        uint64_t x, w, y;
        fscanf(fp, "%llu%llu%llu%llu", &tag, &x, &w, &y);

        mm128_t* p;
        p = &a[i];
        p->x = tag << 32 | x;
        p->y = w <<32 | y;
    }
    //skip_to_EOR(fp);
    return a;
}

void post_chain(int32_t *f, int32_t *p, int32_t *t, int32_t *v, int n) {
    FILE *outfp = fopen("kout4.txt", "w");
    static int count = 0;
    if(count++ > READ_NUM) {
        fclose(outfp);
        exit(0);
    }
    

    fprintf(outfp, "%lld\n", (long long)n);
    fprintf(outfp, "f\tp\tv\tt\n");
    for(int i = 0; i < n; i ++) {
        fprintf(outfp, "%d\t%d\t%d\t%d\n", (int)f[i], (int)p[i], (int)v[i], (int)t[i]);
    }
    fprintf(outfp, "EOR\n");
}

int main() {

    int max_dist_x = 5000;
    int max_dist_y = 5000;
    int bw = 500;
    int max_skip = 25;
    int max_iter = 5000;
    int min_cnt = 3;
    int min_sc = 40;
    int is_cdna = 0;
    int n_segs = 1;
    int64_t n = 0;
    mm128_t *a;
    int32_t *f, *p, *t, *v;
    int *n_u_;
    uint64_t **_u;
    read_t read;
    FILE *infp = fopen("data/in4.txt", "r");
    if(infp == NULL){
        printf("ERROR TO GET FILE!"); 
        return -1;
    }
    mm_tbuf_t *b = (mm_tbuf_t*)calloc(1, sizeof(mm_tbuf_t));
    b->km = calloc(1, sizeof(kmem_t));  // alloc for km

    a = parse_read(b->km, infp, a, &n, &max_dist_x, &max_dist_y, &bw);

    f = (int32_t*)kmalloc(b->km, n * 4);
    p = (int32_t*)kmalloc(b->km, n * 4);
    t = (int32_t*)kmalloc(b->km, n * 4);
    v = (int32_t*)kmalloc(b->km, n * 4);
    memset(t, 0,  n * 4);

    mm_chain_dp(max_dist_x, max_dist_y, bw, max_skip, max_iter, min_cnt, min_sc, is_cdna, n_segs, n, a, n_u_, _u, b->km, f, p, t, v);
    fclose(infp);
    post_chain(f, p, t, v, n);
    return 0;
}
