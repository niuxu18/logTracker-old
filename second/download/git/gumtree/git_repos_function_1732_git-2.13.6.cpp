static char* make_fast_import_path(const char *path)
{
	if (!relative_marks_paths || is_absolute_path(path))
		return xstrdup(path);
	return git_pathdup("info/fast-import/%s", path);
}