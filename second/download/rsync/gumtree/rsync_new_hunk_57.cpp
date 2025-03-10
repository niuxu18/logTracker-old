  fprintf(f,"-p, --perms              preserve permissions\n");
  fprintf(f,"-o, --owner              preserve owner (root only)\n");
  fprintf(f,"-g, --group              preserve group\n");
  fprintf(f,"-D, --devices            preserve devices (root only)\n");
  fprintf(f,"-t, --times              preserve times\n");  
  fprintf(f,"-n, --dry-run            show what would have been transferred\n");
  fprintf(f,"-x, --one-file-system    don't cross filesystem boundaries\n");
  fprintf(f,"-B, --block-size SIZE    checksum blocking size\n");  
  fprintf(f,"-e, --rsh COMMAND        specify rsh replacement\n");
  fprintf(f,"-C, --cvs-exclude        auto ignore files in the same way CVS does\n");
  fprintf(f,"    --delete             delete files that don't exist on the sending side\n");
  fprintf(f,"-I, --ignore-times       don't exclude files that match length and time\n");
  fprintf(f,"    --exclude FILE       exclude file FILE\n");
