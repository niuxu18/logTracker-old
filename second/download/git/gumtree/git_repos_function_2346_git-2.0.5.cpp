static struct rpc_service *select_service(const char *name)
{
	struct rpc_service *svc = NULL;
	int i;

	if (!starts_with(name, "git-"))
		forbidden("Unsupported service: '%s'", name);

	for (i = 0; i < ARRAY_SIZE(rpc_service); i++) {
		struct rpc_service *s = &rpc_service[i];
		if (!strcmp(s->name, name + 4)) {
			svc = s;
			break;
		}
	}

	if (!svc)
		forbidden("Unsupported service: '%s'", name);

	if (svc->enabled < 0) {
		const char *user = getenv("REMOTE_USER");
		svc->enabled = (user && *user) ? 1 : 0;
	}
	if (!svc->enabled)
		forbidden("Service not enabled: '%s'", svc->name);
	return svc;
}