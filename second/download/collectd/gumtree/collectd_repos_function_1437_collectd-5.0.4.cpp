static int
process_stations (int sk, const char *dev)
{
	uint8_t buf[24*1024];
	struct iwreq iwr;
	uint8_t *cp;
	int len, nodes;
	int status;

	memset (&iwr, 0, sizeof (iwr));
	sstrncpy (iwr.ifr_name, dev, sizeof (iwr.ifr_name));
	iwr.u.data.pointer = (void *) buf;
	iwr.u.data.length = sizeof (buf);

	status = ioctl (sk, IEEE80211_IOCTL_STA_INFO, &iwr);
	if (status < 0)
	{
		/* Silent, because not all interfaces support all ioctls. */
		DEBUG ("madwifi plugin: Sending IO-control "
				"IEEE80211_IOCTL_STA_INFO to device %s "
				"failed with status %i.",
				dev, status);
		return (status);
	}

	len = iwr.u.data.length;

	cp = buf;
	nodes = 0;
	while (len >= sizeof (struct ieee80211req_sta_info))
	{
		struct ieee80211req_sta_info *si = (void *) cp;
		process_station(sk, dev, si);
		cp += si->isi_len;
		len -= si->isi_len;
		nodes++;
	}

	if (item_watched (STAT_ATH_NODES))
		submit_gauge (dev, "ath_nodes", NULL, NULL, nodes);
	return (0);
}