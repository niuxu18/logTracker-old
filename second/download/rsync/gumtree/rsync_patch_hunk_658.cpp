 }
 
 static void add_delete_entry(struct file_struct *file)
 {
 	if (dlist_len == dlist_alloc_len) {
 		dlist_alloc_len += 1024;
-		delete_list = (struct delete_list *)Realloc(delete_list, sizeof(delete_list[0])*dlist_alloc_len);
+		delete_list = realloc_array(delete_list, struct delete_list,
+					    dlist_alloc_len);
 		if (!delete_list) out_of_memory("add_delete_entry");
 	}
 
 	delete_list[dlist_len].dev = file->dev;
 	delete_list[dlist_len].inode = file->inode;
 	dlist_len++;
