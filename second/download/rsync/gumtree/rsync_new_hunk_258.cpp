	  if (verbose > 3)
	    fprintf(FERROR,"potential match at %d target=%d %d sum=%08x\n",
		    offset,j,i,sum);

	  if (!done_csum2) {
	    int l = MIN(s->n,len-offset);
	    map = (signed char *)map_ptr(buf,offset,l);
	    get_checksum2((char *)map,l,sum2);
	    done_csum2 = 1;
	  }
	  if (memcmp(sum2,s->sums[i].sum2,csum_length) == 0) {
	    matched(f,s,buf,offset,i);
	    offset += s->sums[i].len - 1;
	    k = MIN((len-offset), s->n);
	    map = (signed char *)map_ptr(buf,offset,k);
	    sum = get_checksum1((char *)map, k);
	    s1 = sum & 0xFFFF;
	    s2 = sum >> 16;
	    ++matches;
	    break;
	  } else {
	    false_alarms++;
