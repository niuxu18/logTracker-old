	len = remainder;

      if (verbose > 3)
	fprintf(FERROR,"chunk[%d] of size %d at %d offset=%d\n",
		i,len,(int)offset2,(int)offset);

      if (write_sparse(fd,map_ptr(buf,offset2,len),len) != len) {
	fprintf(FERROR,"write failed on %s : %s\n",fname,strerror(errno));
	exit_cleanup(1);
      }
      offset += len;
    }
  }

  if (offset > 0 && sparse_end(fd) != 0) {
    fprintf(FERROR,"write failed on %s : %s\n",fname,strerror(errno));
    exit_cleanup(1);
  }
}


static void delete_one(struct file_struct *f)
{
  if (!S_ISDIR(f->mode)) {
