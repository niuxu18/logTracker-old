static int total_tag_hits;
static int total_matches;

extern struct stats stats;

struct target {
	tag t;
	int i;
};

static struct target *targets;

static int *tag_table;

#define gettag2(s1,s2) (((s1) + (s2)) & 0xFFFF)
#define gettag(sum) gettag2((sum)&0xFFFF,(sum)>>16)

static int compare_targets(struct target *t1,struct target *t2)
{
	return (int)t1->t - (int)t2->t;
}


static void build_hash_table(struct sum_struct *s)
{
	int i;

	if (!tag_table)
		tag_table = new_array(int, TABLESIZE);

	targets = new_array(struct target, s->count);
	if (!tag_table || !targets)
		out_of_memory("build_hash_table");

	for (i = 0; i < (int)s->count; i++) {
		targets[i].i = i;
		targets[i].t = gettag(s->sums[i].sum1);
	}

	qsort(targets,s->count,sizeof(targets[0]),(int (*)())compare_targets);

	for (i = 0; i < TABLESIZE; i++)
		tag_table[i] = NULL_TAG;

	for (i = s->count-1; i >= 0; i--)
		tag_table[targets[i].t] = i;
}


static OFF_T last_match;


