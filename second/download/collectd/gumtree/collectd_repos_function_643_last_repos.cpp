static jint JNICALL cjni_api_register_log(JNIEnv *jvm_env, /* {{{ */
                                          jobject this, jobject o_name,
                                          jobject o_log) {
  cjni_callback_info_t *cbi;

  cbi = cjni_callback_info_create(jvm_env, o_name, o_log, CB_TYPE_LOG);
  if (cbi == NULL)
    return -1;

  DEBUG("java plugin: Registering new log callback: %s", cbi->name);

  plugin_register_log(cbi->name, cjni_log,
                      &(user_data_t){
                          .data = cbi, .free_func = cjni_callback_info_destroy,
                      });

  (*jvm_env)->DeleteLocalRef(jvm_env, o_log);

  return 0;
}