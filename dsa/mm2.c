#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define MM_SEED_SEG_SHIFT 48
#define MM_SEED_SEG_MASK (0xffULL << (MM_SEED_SEG_SHIFT))
#define READ_NUM 3000
typedef struct
{
    uint64_t x, y;
}mm128_t;
static const char LogTable256[256] = {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)};

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
                     int max_skip, int max_iter, int min_cnt, int min_sc, int is_cdna, int n_segs, int64_t n, mm128_t *a, int *n_u_, uint64_t **_u, void *km, int32_t *f, int32_t *p, int32_t *t, int32_t *v)
{

    int32_t k, n_u, n_v;
    int64_t st = 0;
    uint64_t *u, *u2, sum_qspan = 0;
    float avg_qspan;
    mm128_t *b, *w;

    if (n == 0 || a == 0) {
        return;
    }
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
}
