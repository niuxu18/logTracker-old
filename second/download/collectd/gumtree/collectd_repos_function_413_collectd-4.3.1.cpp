static int perl_notify (const notification_t *notif)
{
	dTHX;

	if (NULL == perl_threads)
		return 0;

	if (NULL == aTHX) {
		c_ithread_t *t = NULL;

		pthread_mutex_lock (&perl_threads->mutex);
		t = c_ithread_create (perl_threads->head->interp);
		pthread_mutex_unlock (&perl_threads->mutex);

		aTHX = t->interp;
	}
	return pplugin_call_all (aTHX_ PLUGIN_NOTIF, notif);
}