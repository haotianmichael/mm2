#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct{
    uint64_t x, y;     
}mm128_t;
#define MM_SEED_SEG_SHIFT 48
#define MM_SEED_SEG_MASK (0xffULL<<(MM_SEED_SEG_SHIFT))
void *kmalloc(void *km, size_t size);
void kfree(void*km, void *ptr);

void mm_map_frag(const mm_id) {


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
mm128_t *mm_chain_dp(int max_dist_x, int max_dist_y, int bw, 
        int max_skip, int max_iter, int min_cnt, int min_sc, int is_cdna, int n_segs, int64_t n, mm128_t *a, int *n_u_, uint64_t**_u, void *km){

    int32_t k, n_u, n_v;
    int64_t st = 0;
    uint64_t *u, *u2, sum_qspan = 0;
    float avg_qspan;
    mm128_t *b, *w;


    /*if(_u) *_u = 0, *n_u = 0;*/
    if(n == 0 || a == 0) {
        kfree(km, a);
        return 0;
    }


    int32_t *f, *p, *t, *v;
    f = (int32_t*)kmalloc(km, n * 4);
    p = (int32_t*)kmalloc(km, n * 4);;
    t = (int32_t*)kmalloc(km, n * 4);
    v = (int32_t*)kmalloc(km, n * 4);
    memset(t, 0, n * 4);

    //calculate average length of seeds
    for(int i = 0; i < n; i ++) sum_qspan += a[i].y>>32&0xff;
    avg_qspan = (float)sum_qspan / n;


    for(int i = 0; i < n; i ++) {   // iterate all anchors from array
        uint64_t ri = a[i].x;
        int64_t max_j = -1;
        int32_t qi = (int32_t)a[i].y, q_span = a[i].y>>32&0xff;
        int32_t max_f = q_span, n_skip = 0, min_d;
        int32_t sidi = (a[i].y & MM_SEED_SEG_MASK) >> MM_SEED_SEG_SHIFT;
        
        while(st < i && ri > a[st].x + max_dist_x) ++st;
        if(i - st > max_iter)  st = i - max_iter;
        for(int j = i - 1; j >= st; --j) {



        }

    }














}
