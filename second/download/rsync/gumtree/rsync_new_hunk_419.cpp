  for (i=s->count-1;i>=0;i--) {    
    tag_table[targets[i].t] = i;
  }
}


static OFF_T last_match;


static void matched(int f,struct sum_struct *s,struct map_struct *buf,
		    OFF_T offset,int i)
{
	OFF_T n = offset - last_match;
	int j;

	if (verbose > 2 && i >= 0)
		rprintf(FINFO,"match at %d last_match=%d j=%d len=%d n=%d\n",
			(int)offset,(int)last_match,i,(int)s->sums[i].len,(int)n);

	send_token(f,i,buf,last_match,n,i==-1?0:s->sums[i].len);
	data_transfer += n;

	if (n > 0)
