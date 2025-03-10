static void usage(FILE *f)
{
  fprintf(f,"rsync version %s Copyright Andrew Tridgell and Paul Mackerras\n\n",
	  VERSION);
  fprintf(f,"Usage:\t%s [options] src user@host:dest\nOR",RSYNC_NAME);
  fprintf(f,"\t%s [options] user@host:src dest\n\n",RSYNC_NAME);
  fprintf(f,"Options:\n");
  fprintf(f,"-v, --verbose            increase verbosity\n");
  fprintf(f,"-c, --checksum           always checksum\n");
  fprintf(f,"-a, --archive            archive mode (same as -rlptDog)\n");
  fprintf(f,"-r, --recursive          recurse into directories\n");
  fprintf(f,"-b, --backup             make backups (default ~ extension)\n");
  fprintf(f,"-u, --update             update only (don't overwrite newer files)\n");
  fprintf(f,"-l, --links              preserve soft links\n");
  fprintf(f,"-p, --perms              preserve permissions\n");
  fprintf(f,"-o, --owner              preserve owner (root only)\n");
  fprintf(f,"-g, --group              preserve group\n");
  fprintf(f,"-D, --devices            preserve devices (root only)\n");
  fprintf(f,"-t, --times              preserve times\n");  
  fprintf(f,"-n, --dry-run            show what would have been transferred\n");
  fprintf(f,"-x, --one-file-system    don't cross filesystem boundaries\n");
  fprintf(f,"-B, --block-size SIZE    checksum blocking size\n");  
  fprintf(f,"-e, --rsh COMMAND        specify rsh replacement\n");
  fprintf(f,"    --rsync-path PATH    specify path to rsync on the remote machine\n");
  fprintf(f,"-C, --cvs-exclude        auto ignore files in the same way CVS does\n");
  fprintf(f,"    --delete             delete files that don't exist on the sending side\n");
  fprintf(f,"-I, --ignore-times       don't exclude files that match length and time\n");
  fprintf(f,"    --exclude FILE       exclude file FILE\n");
  fprintf(f,"    --exclude-from FILE  exclude files listed in FILE\n");
  fprintf(f,"    --suffix SUFFIX      override backup suffix\n");  
  fprintf(f,"    --csum-length LENGTH set the checksum length\n");  
  fprintf(f,"    --version            print version number\n");  

  fprintf(f,"\n");
  fprintf(f,"the backup suffix defaults to %s\n",BACKUP_SUFFIX);
  fprintf(f,"the block size defaults to %d\n",BLOCK_SIZE);  
}