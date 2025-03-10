/*=========================================================================

Program:   KWSys - Kitware System Library
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
See http://www.cmake.org/HTML/Copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <stdio.h>

int main(int argc, char* argv[])
{
  FILE* ifp;
  FILE* ofp;
  int i;
  int n;
  int count = 0;
  unsigned char buffer[1024];
  
  /* Check arguments.  */
  if(argc != 5)
    {
    fprintf(stderr, "Usage: %s <input> <output> <kwsys-name> <array>\n",
            argv[0]);
    return 1;
    }
  
  /* Open the input file.  */
  ifp = fopen(argv[1], "rb");
  if(!ifp)
    {
    fprintf(stderr, "Cannot open input file: \"%s\"\n", argv[1]);
    return 2;
    }
  ofp = fopen(argv[2], "w");
  if(!ofp)
    {
    fprintf(stderr, "Cannot open output file: \"%s\"\n", argv[2]);
    return 2;
    }
  
  /* Prepend header comment.  */
  fprintf(ofp, "/*\n * DO NOT EDIT\n * This file is generated by:\n");
  fprintf(ofp, " * %s\n */\n\n", argv[0]);
  fprintf(ofp, "#include <stdio.h>\n\n");
  
  /* Split file up in 1024-byte chunks.  */
  while((n = fread(buffer, 1, 1024, ifp)) > 0)
    {
    fprintf(ofp, "static unsigned char kwsysEncodedArray%s_%d[%d] = {\n", 
            argv[4], count++, n);
    for(i=0; i < n-1; ++i)
      {
      fprintf(ofp, "0x%02X", buffer[i]);
      if(i%10 == 9)
        {
        fprintf(ofp, ",\n");
        }
      else
        {
        fprintf(ofp, ", ");
        }
      }
    fprintf(ofp, "0x%02X};\n\n", buffer[n-1]);
    }
  fclose(ifp);
  
  /* Provide a function to write the data to a file.  */
  fprintf(ofp, "extern int %sEncodedWriteArray%s(const char* fname)\n",
          argv[3], argv[4]);
  fprintf(ofp, "{\n");
  fprintf(ofp, "  FILE* ofp = fopen(fname, \"wb\");\n");
  fprintf(ofp, "  if(!ofp) { return 0; }\n");
  for(i=0; i < count; ++i)
    {
    fprintf(ofp, "  if(fwrite(kwsysEncodedArray%s_%d, 1,\n"
                 "            sizeof(kwsysEncodedArray%s_%d), ofp) !=\n"
                 "       sizeof(kwsysEncodedArray%s_%d))\n",
            argv[4], i, argv[4], i, argv[4], i);
    fprintf(ofp, "    {\n");
    fprintf(ofp, "    fclose(ofp);\n");
    fprintf(ofp, "    _unlink(fname);\n");
    fprintf(ofp, "    return 0;\n");
    fprintf(ofp, "    }\n");
    }
  fprintf(ofp, "  fclose(ofp);\n");
  fprintf(ofp, "  return 1;\n");
  fprintf(ofp, "}\n");
  fclose(ofp);
  return 0;
}
