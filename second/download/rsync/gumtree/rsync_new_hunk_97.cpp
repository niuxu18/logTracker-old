	if (sum == s->sums[i].sum1) {
	  if (verbose > 3)
	    fprintf(stderr,"potential match at %d target=%d %d sum=%08x\n",
		    offset,j,i,sum);

	  if (!done_csum2) {
	    int l = MIN(s->n,len-offset);
	    map = map_ptr(buf,offset,l);
	    get_checksum2(map,l,sum2);
	    done_csum2 = 1;
	  }
	  if (memcmp(sum2,s->sums[i].sum2,SUM_LENGTH) == 0) {
	    matched(f,s,buf,len,offset,i);
	    offset += s->sums[i].len - 1;
	    k = MIN((len-offset), s->n);
	    map = map_ptr(buf,offset,k);
	    sum = get_checksum1(map, k);
	    s1 = sum & 0xFFFF;
	    s2 = sum >> 16;
	    ++matches;
	    break;
	  } else {
	    false_alarms++;
