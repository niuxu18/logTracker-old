{
			/* This file has the same offset
			 * but it's wrong offset which empty files
			 * and symlink files have.
			 * NOTE: This wrong offset was recorded by
			 * old mkisofs utility. If ISO images is
			 * created by latest mkisofs, this does not
			 * happen.
			 */
			file->next = NULL;
			*empty_files.last = file;
			empty_files.last = &(file->next);
		}