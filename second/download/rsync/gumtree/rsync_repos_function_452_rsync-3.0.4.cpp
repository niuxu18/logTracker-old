void send_acl(stat_x *sxp, int f)
{
	if (!sxp->acc_acl) {
		sxp->acc_acl = create_racl();
		rsync_acl_fake_perms(sxp->acc_acl, sxp->st.st_mode);
	}
	/* Avoid sending values that can be inferred from other data. */
	rsync_acl_strip_perms(sxp->acc_acl);

	send_rsync_acl(sxp->acc_acl, SMB_ACL_TYPE_ACCESS, &access_acl_list, f);

	if (S_ISDIR(sxp->st.st_mode)) {
		if (!sxp->def_acl)
			sxp->def_acl = create_racl();

		send_rsync_acl(sxp->def_acl, SMB_ACL_TYPE_DEFAULT, &default_acl_list, f);
	}
}