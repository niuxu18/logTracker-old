static int config_file_fgetc(struct config_source *conf)
{
	return fgetc(conf->u.file);
}