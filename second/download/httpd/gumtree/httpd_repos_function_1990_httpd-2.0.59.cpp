static void core_opts_merge(const ap_conf_vector_t *sec, core_opts_t *opts)
{
    core_dir_config *this_dir = ap_get_module_config(sec, &core_module);

    if (!this_dir) {
        return;
    }

    if (this_dir->opts & OPT_UNSET) {
        opts->add = (opts->add & ~this_dir->opts_remove)
                   | this_dir->opts_add;
        opts->remove = (opts->remove & ~this_dir->opts_add)
                      | this_dir->opts_remove;
        opts->opts = (opts->opts & ~opts->remove) | opts->add;
    }
    else {
        opts->opts = this_dir->opts;
        opts->add = this_dir->opts_add;
        opts->remove = this_dir->opts_remove;
    }

    if (!(this_dir->override & OR_UNSET)) {
        opts->override = this_dir->override;
    }
}