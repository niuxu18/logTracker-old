static void setup_des_key(const unsigned char *key_56,
                          struct des_ctx *des)
{
  char key[8];
  extend_key_56_to_64(key_56, key);
  des_set_key(des, (const uint8_t*)key);
}