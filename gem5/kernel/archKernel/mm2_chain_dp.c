#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "kalloc.h"
#include "gem5/m5ops.h"

/*for allocation of memory*/
typedef struct{
    int rep_len, frag_gap;
    void *km;
}mm_tbuf_t;

static const char LogTable256[256] = {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)};

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

#define MM_SEED_SEG_SHIFT 48
#define MM_SEED_SEG_MASK (0xffULL << (MM_SEED_SEG_SHIFT))
#define READ_NUM 3000
void *kmalloc(void *km, size_t size);
void kfree(void *km, void *ptr);

static inline int ilog2_32(uint32_t v)
{
    uint32_t t, tt;
    if ((tt = v >> 16))
        return (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
    return (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
}

/*
 * @= means by default
 * @param           @usage
 max_dist_x      max_chain_gap_ref = 5000
 max_dist_y      max_chain_gap_query = 5000
 bw              opt->bw = 500
 max_skip        opt->max_skip = 25
 max_iter        opt->max_chain_iter = 5000
 min_cnt         opt->min_cnt = 3
 min_sc          opt->min_chain_score = 40
 is_cdna         is_splice = 0
 n_segs          n_segs = 1
 n               n_a = 346  (num of anchors)
 a               a(mm128_t) (array of anchors)
 n_u_            n_regs0
 _u              u
 km              mm_tbuf_t


*/
void mm_chain_dp(int max_dist_x, int max_dist_y, int bw,
                     int max_skip, int max_iter, int min_cnt, int min_sc, int is_cdna, int n_segs, int64_t n, mm128_t *a, int *n_u_, uint64_t **_u, void *km)
{

    int32_t k, n_u, n_v;
    int64_t st = 0;
    uint64_t *u, *u2, sum_qspan = 0;
    float avg_qspan;
    mm128_t *b, *w;

    /*if(_u) *_u = 0, *n_u = 0;*/
    if (n == 0 || a == 0)
    {
        kfree(km, a);
        return;
    }

    int32_t *f, *p, *t, *v;
    f = (int32_t *)kmalloc(km, n * 4);
    p = (int32_t *)kmalloc(km, n * 4);
    t = (int32_t *)kmalloc(km, n * 4);
    v = (int32_t *)kmalloc(km, n * 4);
    memset(t, 0, n * 4);

    // calculate average length of anchors
    for (int i = 0; i < n; i++)
        sum_qspan += a[i].y >> 32 & 0xff;
    avg_qspan = (float)sum_qspan / n;

    /*
     * ri: where anchor[i] locates in ref
     * qi: where anchors[i] locates in read
     * q_span: the length of anchor[i]
     * dr: Reference Gap
     * dq: Query Gap
     * sc: min(y[i]-y[j], x[i]-x[j], q_span)
     *
     * */

    for (int i = 0; i < n; ++i)
    {
        uint64_t ri = a[i].x;
        int64_t max_j = -1;
        int32_t qi = (int32_t)a[i].y, q_span = a[i].y >> 32 & 0xff; // NB: only 8 bits of span is used!!!
        int32_t max_f = q_span, n_skip = 0, min_d;
        int32_t sidi = (a[i].y & MM_SEED_SEG_MASK) >> MM_SEED_SEG_SHIFT;
        while (st < i && ri > a[st].x + max_dist_x) ++st;
        if (i - st > max_iter) st = i - max_iter;
        for (int j = i - 1; j >= st; --j)
        {
            int64_t dr = ri - a[j].x;
            int32_t dq = qi - (int32_t)a[j].y, dd, sc, log_dd;
            int32_t sidj = (a[j].y & MM_SEED_SEG_MASK) >> MM_SEED_SEG_SHIFT;
            if ((sidi == sidj && dr == 0) || dq <= 0) continue; // don't skip if an anchor is used by multiple segments; see below
            if ((sidi == sidj && dq > max_dist_y) || dq > max_dist_x) continue;
            dd = dr > dq ? dr - dq : dq - dr;
            if (sidi == sidj && dd > bw) continue;
            if (n_segs > 1 && !is_cdna && sidi == sidj && dr > max_dist_y) continue;
            min_d = dq < dr ? dq : dr;
            sc = min_d > q_span ? q_span : dq < dr ? dq : dr;
            log_dd = dd ? ilog2_32(dd) : 0;
            if (is_cdna || sidi != sidj)
            {
                int c_log, c_lin;
                c_lin = (int)(dd * .01 * avg_qspan);
                c_log = log_dd;
                if (sidi != sidj && dr == 0) ++sc; // possibly due to overlapping paired ends; give a minor bonus
                else if (dr > dq || sidi != sidj) sc -= c_lin < c_log ? c_lin : c_log;
                else sc -= c_lin + (c_log >> 1);
            } else sc -= (int)(dd * .01 * avg_qspan) + (log_dd >> 1);
            sc += f[j];
            if (sc > max_f) {
                max_f = sc, max_j = j;
                if (n_skip > 0) --n_skip;
            }
            else if (t[j] == i) {
                if (++n_skip > max_skip)
                    break;
            }
            if (p[j] >= 0) t[p[j]] = i;
        }
        f[i] = max_f, p[i] = max_j;
        v[i] = max_j >= 0 && v[max_j] > max_f ? v[max_j] : max_f; // v[] keeps the peak score up to i; f[] is the score ending at i, not always the peak
    }

    FILE *outfp = fopen("kout4.txt", "w"); 
    static int count = 0;
    if(count++ > READ_NUM) {
        fclose(outfp);
        exit(0);
    }

    // dump chain output after kernel
    fprintf(outfp, "%lld\n", (long long)n);
    fprintf(outfp, "f\tp\tv\tt\n");
    for (int i = 0; i < n; i++) {
        fprintf(outfp, "%d\t%d\t%d\t%d\n", (int)f[i], (int)p[i], (int)v[i], (int)t[i]);
    }
    fprintf(outfp, "EOR\n");

}

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
    int t = fscanf(fp, "%lld%f%d%d%d", n, &avg_qspan, max_dist_x, max_dist_y, bw);

    
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

//    skip_to_EOR(fp);
    return a;
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
    int *n_u_;
    uint64_t **_u;
    read_t read;
    FILE *infp = fopen("in4.txt", "r");
    if(infp == NULL){
        printf("ERROR TO GET FILE!"); 
        return -1;
    }
    mm_tbuf_t *b = (mm_tbuf_t*)calloc(1, sizeof(mm_tbuf_t));
    b->km = calloc(1, sizeof(kmem_t));  // alloc for km

    a = parse_read(b->km, infp, a, &n, &max_dist_x, &max_dist_y, &bw);
    //Starting ROI of Gem5
#ifdef GEM5
    m5_work_begin(0, 0);
#endif
    mm_chain_dp(max_dist_x, max_dist_y, bw, max_skip, max_iter, min_cnt, min_sc, is_cdna, n_segs, n, a, n_u_, _u, b->km);
#ifdef GEM5
    m5_work_end(0, 0);
#endif
    // Ending ROI of Gem5
    fclose(infp);
    return 0;
}
