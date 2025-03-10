static int lpar_read (void)
{
	perfstat_partition_total_t lparstats;
	int status;
	struct utsname name;
	u_longlong_t ticks;
	u_longlong_t user_ticks, syst_ticks, wait_ticks, idle_ticks;
	u_longlong_t consumed_ticks;
	double entitled_proc_capacity;

	/* An LPAR has the same serial number as the physical system it is currently
	   running on. It is a convenient way of tracking LPARs as they are moved
	   from chassis to chassis through Live Partition Mobility (LPM). */
	if (uname (&name) != 0)
	{
		ERROR ("lpar plugin: uname failed.");
		return (-1);
	}
	sstrncpy (serial, name.machine, sizeof (serial));

	/* Retrieve the current metrics. Returns the number of structures filled. */
	status = perfstat_partition_total (/* name = */ NULL, /* (must be NULL) */
			&lparstats, sizeof (perfstat_partition_total_t),
			/* number = */ 1 /* (must be 1) */);
	if (status != 1)
	{
		char errbuf[1024];
		ERROR ("lpar plugin: perfstat_partition_total failed: %s (%i)",
				sstrerror (errno, errbuf, sizeof (errbuf)),
				status);
		return (-1);
	}

	/* Number of ticks since we last run. */
	ticks = lparstats.timebase_last - lparstats_old.timebase_last;
	if (ticks == 0)
	{
		/* The stats have not been updated. Return now to avoid
		 * dividing by zero */
		return (0);
	}

	/*
	 * On a shared partition, we're "entitled" to a certain amount of
	 * processing power, for example 250/100 of a physical CPU. Processing
	 * capacity not used by the partition may be assigned to a different
	 * partition by the hypervisor, so "idle" is hopefully a very small
	 * number.
	 *
	 * A dedicated partition may donate its CPUs to another partition and
	 * may steal ticks from somewhere else (another partition or maybe the
	 * shared pool, I don't know --octo).
	 */

	/* entitled_proc_capacity is in 1/100th of a CPU */
	entitled_proc_capacity = 0.01 * ((double) lparstats.entitled_proc_capacity);
	lpar_submit ("entitled", entitled_proc_capacity);

	/* The number of ticks actually spent in the various states */
	user_ticks = lparstats.puser - lparstats_old.puser;
	syst_ticks = lparstats.psys  - lparstats_old.psys;
	wait_ticks = lparstats.pwait - lparstats_old.pwait;
	idle_ticks = lparstats.pidle - lparstats_old.pidle;
	consumed_ticks = user_ticks + syst_ticks + wait_ticks + idle_ticks;

	lpar_submit ("user", (double) user_ticks / (double) ticks);
	lpar_submit ("system", (double) syst_ticks / (double) ticks);
	lpar_submit ("wait", (double) wait_ticks / (double) ticks);
	lpar_submit ("idle", (double) idle_ticks / (double) ticks);

#if PERFSTAT_SUPPORTS_DONATION
	if (donate_flag)
	{
		/* donated => ticks given to another partition
		 * stolen  => ticks received from another partition */
		u_longlong_t idle_donated_ticks, busy_donated_ticks;
		u_longlong_t idle_stolen_ticks, busy_stolen_ticks;

		/* FYI:  PURR == Processor Utilization of Resources Register
		 *      SPURR == Scaled PURR */
		idle_donated_ticks = lparstats.idle_donated_purr - lparstats_old.idle_donated_purr;
		busy_donated_ticks = lparstats.busy_donated_purr - lparstats_old.busy_donated_purr;
		idle_stolen_ticks  = lparstats.idle_stolen_purr  - lparstats_old.idle_stolen_purr;
		busy_stolen_ticks  = lparstats.busy_stolen_purr  - lparstats_old.busy_stolen_purr;

		lpar_submit ("idle_donated", (double) idle_donated_ticks / (double) ticks);
		lpar_submit ("busy_donated", (double) busy_donated_ticks / (double) ticks);
		lpar_submit ("idle_stolen",  (double) idle_stolen_ticks  / (double) ticks);
		lpar_submit ("busy_stolen",  (double) busy_stolen_ticks  / (double) ticks);

		/* Donated ticks will be accounted for as stolen ticks in other LPARs */
		consumed_ticks += idle_stolen_ticks + busy_stolen_ticks;
	}
#endif

	lpar_submit ("consumed", (double) consumed_ticks / (double) ticks);

	if (pool_stats)
	{
		char typinst[DATA_MAX_NAME_LEN];
		u_longlong_t pool_idle_cticks;
		double pool_idle_cpus;
		double pool_busy_cpus;

		/* We're calculating "busy" from "idle" and the total number of
		 * CPUs, because the "busy" member didn't exist in early versions
		 * of libperfstat. It was added somewhere between AIX 5.3 ML5 and ML9. */
		pool_idle_cticks = lparstats.pool_idle_time - lparstats_old.pool_idle_time;
		pool_idle_cpus = CLOCKTICKS_TO_TICKS ((double) pool_idle_cticks) / (double) ticks;
		pool_busy_cpus = ((double) lparstats.phys_cpus_pool) - pool_idle_cpus;
		if (pool_busy_cpus < 0.0)
			pool_busy_cpus = 0.0;

		ssnprintf (typinst, sizeof (typinst), "pool-%X-busy", lparstats.pool_id);
		lpar_submit (typinst, pool_busy_cpus);

		ssnprintf (typinst, sizeof (typinst), "pool-%X-idle", lparstats.pool_id);
		lpar_submit (typinst, pool_idle_cpus);
	}

	memcpy (&lparstats_old, &lparstats, sizeof (lparstats_old));

	return (0);
}