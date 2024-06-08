/*
 * Test example for branch accuracy and functionality, originally
 * provided by Timothy Kaiser, SDSC. It was modified to fit the
 * PAPI test suite by Nils Smeds, <smeds@pdc.kth.se>.
 * and Phil Mucci <mucci@cs.utk.edu>
 * This example verifies the accuracy of branch events
 */

/* Measures 4 events:
	PAPI_BR_NTK -- branches not taken
	PAPI_BR_PRC -- branches predicted correctly
	PAPI_BR_INS -- total branch instructions
	PAPI_BR_MSP -- branches mispredicted
  First measure all 4 at once (or as many as will fit).
  Then run them one by one.
  Compare results to see if they match.

  Note: sometimes have seen failure if system is under fuzzing load

*/
/*
 *
 * This version of papi branchse is modified to profile mm2's chaining function.
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stddef.h> /* for size_t */

#include "papi.h"
#include "papi_test.h"

#define MAXEVENTS 4
#define MINCOUNTS 100000
#define MPX_TOLERANCE .20


typedef struct header_t {
	size_t size;
	struct header_t *ptr;
} header_t;

typedef struct {
	header_t base, *loop_head, *core_head; /* base is a zero-sized block always kept in the loop */
} kmem_t;


typedef struct {
	size_t capacity, available, n_blocks, n_cores, largest;
} km_stat_t;



#define MIN_CORE_SIZE 0x80000

static void panic(const char *s)
{
	fprintf(stderr, "%s\n", s);
	abort();
}

void *km_init(void)
{
	return calloc(1, sizeof(kmem_t));
}

void km_destroy(void *_km)
{
	kmem_t *km = (kmem_t*)_km;
	header_t *p, *q;
	if (km == NULL) return;
	for (p = km->core_head; p != NULL;) {
		q = p->ptr;
		free(p);
		p = q;
	}
	free(km);
}

static header_t *morecore(kmem_t *km, size_t nu)
{
	header_t *q;
	size_t bytes, *p;
	nu = (nu + 1 + (MIN_CORE_SIZE - 1)) / MIN_CORE_SIZE * MIN_CORE_SIZE; /* the first +1 for core header */
	bytes = nu * sizeof(header_t);
	q = (header_t*)malloc(bytes);
	if (!q) panic("[morecore] insufficient memory");
	q->ptr = km->core_head, q->size = nu, km->core_head = q;
	p = (size_t*)(q + 1);
	*p = nu - 1; /* the size of the free block; -1 because the first unit is used for the core header */
	kfree(km, p + 1); /* initialize the new "core"; NB: the core header is not looped. */
	return km->loop_head;
}

void kfree(void *_km, void *ap) /* kfree() also adds a new core to the circular list */
{
	header_t *p, *q;
	kmem_t *km = (kmem_t*)_km;
	
	if (!ap) return;
	if (km == NULL) {
		free(ap);
		return;
	}
	p = (header_t*)((size_t*)ap - 1);
	p->size = *((size_t*)ap - 1);
	/* Find the pointer that points to the block to be freed. The following loop can stop on two conditions:
	 *
	 * a) "p>q && p<q->ptr": @------#++++++++#+++++++@-------    @---------------#+++++++@-------
	 *    (can also be in    |      |                |        -> |                       |
	 *     two cores)        q      p           q->ptr           q                  q->ptr
	 *
	 *                       @--------    #+++++++++@--------    @--------    @------------------
	 *                       |            |         |         -> |            |
	 *                       q            p    q->ptr            q       q->ptr
	 *
	 * b) "q>=q->ptr && (p>q || p<q->ptr)":  @-------#+++++   @--------#+++++++     @-------#+++++   @----------------
	 *                                       |                |        |         -> |                |
	 *                                  q->ptr                q        p       q->ptr                q
	 *
	 *                                       #+++++++@-----   #++++++++@-------     @-------------   #++++++++@-------
	 *                                       |       |                 |         -> |                         |
	 *                                       p  q->ptr                 q       q->ptr                         q
	 */
	for (q = km->loop_head; !(p > q && p < q->ptr); q = q->ptr)
		if (q >= q->ptr && (p > q || p < q->ptr)) break;
	if (p + p->size == q->ptr) { /* two adjacent blocks, merge p and q->ptr (the 2nd and 4th cases) */
		p->size += q->ptr->size;
		p->ptr = q->ptr->ptr;
	} else if (p + p->size > q->ptr && q->ptr >= p) {
		panic("[kfree] The end of the allocated block enters a free block.");
	} else p->ptr = q->ptr; /* backup q->ptr */

	if (q + q->size == p) { /* two adjacent blocks, merge q and p (the other two cases) */
		q->size += p->size;
		q->ptr = p->ptr;
		km->loop_head = q;
	} else if (q + q->size > p && p >= q) {
		panic("[kfree] The end of a free block enters the allocated block.");
	} else km->loop_head = p, q->ptr = p; /* in two cores, cannot be merged; create a new block in the list */
}

void *kmalloc(void *_km, size_t n_bytes)
{
	kmem_t *km = (kmem_t*)_km;
	size_t n_units;
	header_t *p, *q;

	if (n_bytes == 0) return 0;
	if (km == NULL) return malloc(n_bytes);
	n_units = (n_bytes + sizeof(size_t) + sizeof(header_t) - 1) / sizeof(header_t) + 1;

	if (!(q = km->loop_head)) /* the first time when kmalloc() is called, intialize it */
		q = km->loop_head = km->base.ptr = &km->base;
	for (p = q->ptr;; q = p, p = p->ptr) { /* search for a suitable block */
		if (p->size >= n_units) { /* p->size if the size of current block. This line means the current block is large enough. */
			if (p->size == n_units) q->ptr = p->ptr; /* no need to split the block */
			else { /* split the block. NB: memory is allocated at the end of the block! */
				p->size -= n_units; /* reduce the size of the free block */
				p += p->size; /* p points to the allocated block */
				*(size_t*)p = n_units; /* set the size */
			}
			km->loop_head = q; /* set the end of chain */
			return (size_t*)p + 1;
		}
		if (p == km->loop_head) { /* then ask for more "cores" */
			if ((p = morecore(km, n_units)) == 0) return 0;
		}
	}
}

void *kcalloc(void *_km, size_t count, size_t size)
{
	kmem_t *km = (kmem_t*)_km;
	void *p;
	if (size == 0 || count == 0) return 0;
	if (km == NULL) return calloc(count, size);
	p = kmalloc(km, count * size);
	memset(p, 0, count * size);
	return p;
}

void *krealloc(void *_km, void *ap, size_t n_bytes) // TODO: this can be made more efficient in principle
{
	kmem_t *km = (kmem_t*)_km;
	size_t n_units, *p, *q;

	if (n_bytes == 0) {
		kfree(km, ap); return 0;
	}
	if (km == NULL) return realloc(ap, n_bytes);
	if (ap == NULL) return kmalloc(km, n_bytes);
	n_units = (n_bytes + sizeof(size_t) + sizeof(header_t) - 1) / sizeof(header_t);
	p = (size_t*)ap - 1;
	if (*p >= n_units) return ap; /* TODO: this prevents shrinking */
	q = (size_t*)kmalloc(km, n_bytes);
	memcpy(q, ap, (*p - 1) * sizeof(header_t));
	kfree(km, ap);
	return q;
}

void km_stat(const void *_km, km_stat_t *s)
{
	kmem_t *km = (kmem_t*)_km;
	header_t *p;
	memset(s, 0, sizeof(km_stat_t));
	if (km == NULL || km->loop_head == NULL) return;
	for (p = km->loop_head;; p = p->ptr) {
		s->available += p->size * sizeof(header_t);
		if (p->size != 0) ++s->n_blocks; /* &kmem_t::base is always one of the cores. It is zero-sized. */
		if (p->ptr > p && p + p->size > p->ptr)
			panic("[km_stat] The end of a free block enters another free block.");
		if (p->ptr == km->loop_head) break;
	}
	for (p = km->core_head; p != NULL; p = p->ptr) {
		size_t size = p->size * sizeof(header_t);
		++s->n_cores;
		s->capacity += size;
		s->largest = s->largest > size? s->largest : size;
	}
}


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

    FILE *outfp = fopen("resulttest/kout4.txt", "w"); 
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


int
main( int argc, char **argv )
{
	PAPI_event_info_t info;
	int i, j, retval, errors=0;
	int iters = 10000000;
	double x = 1.1, y;
	long long t1, t2;
	long long values[MAXEVENTS], refvalues[MAXEVENTS];
	double spread[MAXEVENTS];
	int nevents = MAXEVENTS;
	int eventset = PAPI_NULL;
	int events[MAXEVENTS];
	int quiet;
	char event_names[MAXEVENTS][256] = {
		"PAPI_BR_NTK",	// not taken
		"PAPI_BR_PRC",	// predicted correctly
		"PAPI_BR_INS",	// total branches
		"PAPI_BR_MSP",	// branches mispredicted
	};

	/* Set quiet variable */
	quiet = tests_quiet( argc, argv );

	/* Parse command line args */
	if ( argc > 1 ) {
		if ( !strcmp( argv[1], "TESTS_QUIET" ) ) {

		}
	}

	events[0] = PAPI_BR_NTK;	// not taken
	events[1] = PAPI_BR_PRC;	// predicted correctly
	events[2] = PAPI_BR_INS;	// total branches
	events[3] = PAPI_BR_MSP;	// branches mispredicted

	/* Why were these disabled?
	events[3]=PAPI_BR_CN;
	events[4]=PAPI_BR_UCN;
	events[5]=PAPI_BR_TKN; */


	/* Clear out the results to zero */
	for ( i = 0; i < MAXEVENTS; i++ ) {
		values[i] = 0;
	}

	if ( !quiet ) {
		printf( "\nAccuracy check of branch presets.\n" );
		printf( "Comparing a measurement with separate measurements.\n\n" );
	}

	/* Initialize library */
	retval = PAPI_library_init( PAPI_VER_CURRENT );
	if (retval != PAPI_VER_CURRENT ) {
		test_fail( __FILE__, __LINE__, "PAPI_library_init", retval );
	}

	/* Create Eventset */
	retval = PAPI_create_eventset( &eventset );
	if ( retval ) {
		test_fail( __FILE__, __LINE__, "PAPI_create_eventset", retval );
	}

#ifdef MPX
	retval = PAPI_multiplex_init(  );
	if ( retval ) {
		test_fail( __FILE__, __LINE__, "PAPI_multiplex_init", retval );
	}

	retval = PAPI_set_multiplex( eventset );
	if ( retval ) {
		test_fail( __FILE__, __LINE__, "PAPI_set_multiplex", retval );
	}
#endif

	nevents = 0;

	/* Add as many of the 4 events that exist on this machine */
	for ( i = 0; i < MAXEVENTS; i++ ) {
		if ( PAPI_query_event( events[i] ) != PAPI_OK )
			continue;
		if ( PAPI_add_event( eventset, events[i] ) == PAPI_OK ) {
			events[nevents] = events[i];
			nevents++;
		}
	}

	/* If none of the events can be added, skip this test */
	if ( nevents < 1 ) {
		test_skip( __FILE__, __LINE__, "Not enough events left...", 0 );
	}

	/* Find a reasonable number of iterations (each
	 * event active 20 times) during the measurement
	 */

	/* Target: 10000 usec/multiplex, 20 repeats */
	t2 = (long long)(10000 * 20) * nevents;

	if ( t2 > 30e6 ) {
		test_skip( __FILE__, __LINE__, "This test takes too much time",
				   retval );
	}

  /*
   * Preparation for mm2
   * */

    int max_dist_x = 5000;
    int max_dist_y = 5000;
    int bw = 500;
    int max_skip = 25;
    int max_iter = 5000;
    int min_cnt = 3;
    int is_cdna = 0;
    int min_sc = 40;
    int n_segs = 1;
    int64_t n = 0;
    mm128_t *a;
    int *n_u_;
    uint64_t **_u;
    read_t read;
    FILE *infp = fopen("", "r");
    if(infp == NULL) {
        printf("ERROR  TO GET FILE."); 
        return -1;
    } 
    mm_tbuf_t *b = (mm_tbuf_t*)calloc(1, sizeof(mm_tbuf_t));
    b->km = calloc(1, sizeof(kmem_t));
    a = parse_read(b->km, infp, a, &n, &max_dist_x, &max_dist_y, &bw);
    

	/* Measure one run */
	t1 = PAPI_get_real_usec(  );
  mm2_chain_dp(max_dist_x, max_dist_y, bw, max_skip, max_iter, min_cnt, min_sc, is_cdna, n_segs, n, a, n_u_, _u, b->km);
	/*y = do_flops3( x, iters, 1 );*/
	t1 = PAPI_get_real_usec(  ) - t1;

	if ( t2 > t1 )			 /* Scale up execution time to match t2 */
		iters = iters * ( int ) ( t2 / t1 );
	else if ( t1 > 30e6 )	 /* Make sure execution time is < 30s per repeated test */
		test_skip( __FILE__, __LINE__, "This test takes too much time",
				   retval );

	x = 1.0;

	/**********************************/
	/* First run: Grouped Measurement */
	/**********************************/
	if ( !quiet ) {
		printf( "\nFirst run: Together.\n" );
	}

	t1 = PAPI_get_real_usec(  );

	retval = PAPI_start( eventset );
	if (retval) test_fail( __FILE__, __LINE__, "PAPI_start", retval );

  mm2_chain_dp(max_dist_x, max_dist_y, bw, max_skip, max_iter, min_cnt, min_sc, is_cdna, n_segs, n, a, n_u_, _u, b->km);
	/*y = do_flops3( x, iters, 1 );*/

	retval = PAPI_stop( eventset, values );
	if (retval) test_fail( __FILE__, __LINE__, "PAPI_stop", retval );

	t2 = PAPI_get_real_usec(  );

	if ( !quiet ) {
		printf( "\tOperations= %.1f Mflop", y * 1e-6 );
		printf( "\t(%g Mflop/s)\n\n", ( y / ( double ) ( t2 - t1 ) ) );
		printf( "PAPI grouped measurement:\n" );

		for ( j = 0; j < nevents; j++ ) {
			PAPI_get_event_info( events[j], &info );
			printf( "%20s = ", info.short_descr );
			printf( LLDFMT, values[j] );
			printf( "\n" );
		}
		printf( "\n" );
	}

	/* Remove all the events, start again */
	retval = PAPI_remove_events( eventset, events, nevents );
	if (retval) test_fail( __FILE__, __LINE__, "PAPI_remove_events", retval );

	retval = PAPI_destroy_eventset( &eventset );
	if (retval) test_fail( __FILE__, __LINE__, "PAPI_destroy_eventset", retval );

	/* Recreate eventset */
	eventset = PAPI_NULL;
	retval = PAPI_create_eventset( &eventset );
	if (retval) test_fail( __FILE__, __LINE__, "PAPI_create_eventset", retval );

	/* Run events one by one */
	for ( i = 0; i < nevents; i++ ) {

		/* Clear out old event */
		retval = PAPI_cleanup_eventset( eventset );
		if (retval) test_fail( __FILE__, __LINE__, "PAPI_cleanup_eventset", retval );
		/* Add the event */
		retval = PAPI_add_event( eventset, events[i] );
		if (retval) test_fail( __FILE__, __LINE__, "PAPI_add_event", retval );

		x = 1.0;

		if ( !quiet ) {
			printf( "\nReference measurement %d (of %d):\n", i + 1, nevents );
		}

		t1 = PAPI_get_real_usec(  );

		retval = PAPI_start( eventset );
		if (retval) test_fail( __FILE__, __LINE__, "PAPI_start", retval );
   mm2_chain_dp(max_dist_x, max_dist_y, bw, max_skip, max_iter, min_cnt, min_sc, is_cdna, n_segs, n, a, n_u_, _u, b->km);
		/*y = do_flops3( x, iters, 1 );*/

		retval = PAPI_stop( eventset, &refvalues[i] );
		if (retval) test_fail( __FILE__, __LINE__, "PAPI_stop", retval );

		t2 = PAPI_get_real_usec(  );


		if ( !quiet ) {
			printf( "\tOperations= %.1f Mflop", y * 1e-6 );
			printf( "\t(%g Mflop/s)\n\n", ( y / ( double ) ( t2 - t1 ) ) );
			PAPI_get_event_info( events[i], &info );
			printf( "PAPI results:\n%20s = ", info.short_descr );
			printf( LLDFMT, refvalues[i] );
			printf( "\n" );
		}
	}

	if ( !quiet ) {
		printf( "\n" );
	}

	/* Validate the results */

	if ( !quiet ) {
		printf( "\n\nRelative accuracy:\n" );
		printf( "\tEvent\t\tGroup\t\tIndividual\tSpread\n");
	}

	for ( j = 0; j < nevents; j++ ) {
		spread[j] = abs( ( int ) ( refvalues[j] - values[j] ) );
		if ( values[j] )
			spread[j] /= ( double ) values[j];
		if ( !quiet ) {
			printf( "\t%02d: ",j);
			printf( "%s",event_names[j]);
			printf( "\t%10lld", values[j] );
			printf( "\t%10lld", refvalues[j] );
			printf("\t%10.3g\n", spread[j] );
		}

		/* Make sure that NaN get counted as errors */
		if ( spread[j] > MPX_TOLERANCE ) {

			/* Neglect inprecise results with low counts */
			if ( refvalues[j] < MINCOUNTS ) {
			}
			else {
				errors++;
				if (!quiet) {
					printf("\tError: Spread > %lf\n",MPX_TOLERANCE);
				}
			}
		}
	}
	if ( !quiet ) {
		printf( "\n\n" );
	}

	if ( errors ) {
		test_fail( __FILE__, __LINE__, "Values outside threshold", i );
	}

	test_pass( __FILE__ );

	return 0;
}


