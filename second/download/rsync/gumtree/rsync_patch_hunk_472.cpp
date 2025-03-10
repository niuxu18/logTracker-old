   return(len);
 }
 
 
 struct map_struct *map_file(int fd,OFF_T len)
 {
-  struct map_struct *ret;
-  ret = (struct map_struct *)malloc(sizeof(*ret));
-  if (!ret) out_of_memory("map_file");
-
-  ret->map = NULL;
-  ret->fd = fd;
-  ret->size = len;
-  ret->p = NULL;
-  ret->p_size = 0;
-  ret->p_offset = 0;
-  ret->p_len = 0;
+	struct map_struct *ret;
+	ret = (struct map_struct *)malloc(sizeof(*ret));
+	if (!ret) out_of_memory("map_file");
+
+	ret->map = NULL;
+	ret->fd = fd;
+	ret->size = len;
+	ret->p = NULL;
+	ret->p_size = 0;
+	ret->p_offset = 0;
+	ret->p_len = 0;
 
 #ifdef HAVE_MMAP
-  if (len < MAX_MAP_SIZE) {
-	  ret->map = (char *)mmap(NULL,len,PROT_READ,MAP_SHARED,fd,0);
-	  if (ret->map == (char *)-1) {
-		  ret->map = NULL;
-	  }
-  }
+	len = MIN(len, MAX_MAP_SIZE);
+	ret->map = (char *)do_mmap(NULL,len,PROT_READ,MAP_SHARED,fd,0);
+	if (ret->map == (char *)-1) {
+		ret->map = NULL;
+	} else {
+		ret->p_len = len;
+	}
 #endif
-  return ret;
+	return ret;
 }
 
 
 char *map_ptr(struct map_struct *map,OFF_T offset,int len)
 {
 	int nread;
 
-	if (map->map)
-		return map->map+offset;
-
 	if (len == 0) 
 		return NULL;
 
 	if (len > (map->size-offset))
 		len = map->size-offset;
 
+#ifdef HAVE_MMAP
+	if (map->map) {
+		if (offset >= map->p_offset && 
+		    offset+len <= map->p_offset+map->p_len) {
+			return (map->map + (offset - map->p_offset));
+		}
+		if (munmap(map->map, map->p_len) != 0) {
+			rprintf(FERROR,"munmap failed : %s\n", strerror(errno));
+			exit_cleanup(1);
+		}
+
+		/* align the mmap region on a nice boundary back a bit from
+		   where it is asked for to allow for some seeking */
+		if (offset > 2*CHUNK_SIZE) {
+			map->p_offset = offset - 2*CHUNK_SIZE;
+			map->p_offset &= ~((OFF_T)(CHUNK_SIZE-1));
+		} else {
+			map->p_offset = 0;
+		}
+		
+		/* map up to MAX_MAP_SIZE */
+		map->p_len = MAX(len, MAX_MAP_SIZE);
+		map->p_len = MIN(map->p_len, map->size - map->p_offset);
+
+		map->map = (char *)do_mmap(NULL,map->p_len,PROT_READ,
+					   MAP_SHARED,map->fd,map->p_offset);
+
+		if (map->map == (char *)-1) {
+			map->map = NULL;
+			map->p_len = 0;
+			map->p_offset = 0;
+		} else {
+			return (map->map + (offset - map->p_offset));
+		}
+	}
+#endif
+
 	if (offset >= map->p_offset && 
 	    offset+len <= map->p_offset+map->p_len) {
 		return (map->p + (offset - map->p_offset));
 	}
 
 	len = MAX(len,CHUNK_SIZE);
