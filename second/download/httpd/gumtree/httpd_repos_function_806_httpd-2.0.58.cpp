static void shmcb_set_safe_time_ex(unsigned char *dest,
				const unsigned char *src)
{
    memcpy(dest, src, sizeof(time_t));
}