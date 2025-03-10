static void get_color(const char *def_color)
{
	get_color_found = 0;
	parsed_color[0] = '\0';
	git_config_with_options(git_get_color_config, NULL,
				&given_config_source, respect_includes);

	if (!get_color_found && def_color)
		color_parse(def_color, "command line", parsed_color);

	fputs(parsed_color, stdout);
}