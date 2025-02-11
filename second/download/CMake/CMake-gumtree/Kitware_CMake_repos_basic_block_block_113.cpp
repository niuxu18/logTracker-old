{
      static char buffer[SMALL_BUFFER_SIZE + 1];
      FIELD *field;
      bool large_buffer;
      bool cursor_moved = FALSE;
      char *bp;
      char *found_content;
      int pos;

      field = form->current;
      assert(form->w != 0);

      large_buffer = (field->cols > SMALL_BUFFER_SIZE);
      if (large_buffer)
	bp = (char *)malloc((size_t)(field->cols) + 1);
      else
	bp = buffer;

      assert(bp != 0);
      
      if (Single_Line_Field(field))
	{
	  int check_len;

	  pos = form->begincol + field->cols;
	  while (pos < field->dcols)
	    {
	      check_len = field->dcols - pos;
	      if ( check_len >= field->cols )
		check_len = field->cols;
	      cursor_moved = TRUE;
	      wmove(form->w,0,pos);
	      winnstr(form->w,bp,check_len);
	      found_content = 
		After_Last_Non_Pad_Position(bp,check_len,field->pad);
	      if (found_content==bp)
		  pos += field->cols;		  
	      else
		{
		  result = TRUE;
		  break;
		}
	    }
	}
      else
	{
	  pos = form->toprow + field->rows;
	  while (pos < field->drows)
	    {
	      cursor_moved = TRUE;
	      wmove(form->w,pos,0);
	      pos++;
	      winnstr(form->w,bp,field->cols);
	      found_content = 
		After_Last_Non_Pad_Position(bp,field->cols,field->pad);
	      if (found_content!=bp)
		{
		  result = TRUE;
		  break;
		}
	    }
	}

      if (large_buffer)
	free(bp);

      if (cursor_moved)
	wmove(form->w,form->currow,form->curcol);
    }