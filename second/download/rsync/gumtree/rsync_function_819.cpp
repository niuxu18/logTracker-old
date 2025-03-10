static void msg_list_add(int code, char *buf, int len)
{
	struct msg_list_item *ml;

	if (!(ml = new(struct msg_list_item)))
		out_of_memory("msg_list_add");
	ml->next = NULL;
	if (!(ml->buf = new_array(char, len+4)))
		out_of_memory("msg_list_add");
	SIVAL(ml->buf, 0, ((code+MPLEX_BASE)<<24) | len);
	memcpy(ml->buf+4, buf, len);
	ml->len = len+4;
	if (msg_list.tail)
		msg_list.tail->next = ml;
	else
		msg_list.head = ml;
	msg_list.tail = ml;
}