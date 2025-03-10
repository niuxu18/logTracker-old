static int cpu_read (void)
{
#if PROCESSOR_CPU_LOAD_INFO || PROCESSOR_TEMPERATURE
	int cpu;

	kern_return_t status;
	
#if PROCESSOR_CPU_LOAD_INFO
	processor_cpu_load_info_data_t cpu_info;
	mach_msg_type_number_t         cpu_info_len;
#endif
#if PROCESSOR_TEMPERATURE
	processor_info_data_t          cpu_temp;
	mach_msg_type_number_t         cpu_temp_len;
#endif

	host_t cpu_host;

	for (cpu = 0; cpu < cpu_list_len; cpu++)
	{
#if PROCESSOR_CPU_LOAD_INFO
		cpu_host = 0;
		cpu_info_len = PROCESSOR_BASIC_INFO_COUNT;

		if ((status = processor_info (cpu_list[cpu],
						PROCESSOR_CPU_LOAD_INFO, &cpu_host,
						(processor_info_t) &cpu_info, &cpu_info_len)) != KERN_SUCCESS)
		{
			ERROR ("cpu plugin: processor_info failed with status %i", (int) status);
			continue;
		}

		if (cpu_info_len < CPU_STATE_MAX)
		{
			ERROR ("cpu plugin: processor_info returned only %i elements..", cpu_info_len);
			continue;
		}

		submit (cpu, "user", (counter_t) cpu_info.cpu_ticks[CPU_STATE_USER]);
		submit (cpu, "nice", (counter_t) cpu_info.cpu_ticks[CPU_STATE_NICE]);
		submit (cpu, "system", (counter_t) cpu_info.cpu_ticks[CPU_STATE_SYSTEM]);
		submit (cpu, "idle", (counter_t) cpu_info.cpu_ticks[CPU_STATE_IDLE]);
#endif /* PROCESSOR_CPU_LOAD_INFO */
#if PROCESSOR_TEMPERATURE
		/*
		 * Not all Apple computers do have this ability. To minimize
		 * the messages sent to the syslog we do an exponential
		 * stepback if `processor_info' fails. We still try ~once a day
		 * though..
		 */
		if (cpu_temp_retry_counter > 0)
		{
			cpu_temp_retry_counter--;
			continue;
		}

		cpu_temp_len = PROCESSOR_INFO_MAX;

		status = processor_info (cpu_list[cpu],
				PROCESSOR_TEMPERATURE,
				&cpu_host,
				cpu_temp, &cpu_temp_len);
		if (status != KERN_SUCCESS)
		{
			ERROR ("cpu plugin: processor_info failed: %s",
					mach_error_string (status));

			cpu_temp_retry_counter = cpu_temp_retry_step;
			cpu_temp_retry_step *= 2;
			if (cpu_temp_retry_step > cpu_temp_retry_max)
				cpu_temp_retry_step = cpu_temp_retry_max;

			continue;
		}

		if (cpu_temp_len != 1)
		{
			DEBUG ("processor_info (PROCESSOR_TEMPERATURE) returned %i elements..?",
				       	(int) cpu_temp_len);
			continue;
		}

		cpu_temp_retry_counter = 0;
		cpu_temp_retry_step    = 1;

		DEBUG ("cpu_temp = %i", (int) cpu_temp);
#endif /* PROCESSOR_TEMPERATURE */
	}
/* #endif PROCESSOR_CPU_LOAD_INFO */

#elif defined(KERNEL_LINUX)
	int cpu;
	counter_t user, nice, syst, idle;
	counter_t wait, intr, sitr; /* sitr == soft interrupt */
	FILE *fh;
	char buf[1024];

	char *fields[9];
	int numfields;

	if ((fh = fopen ("/proc/stat", "r")) == NULL)
	{
		char errbuf[1024];
		ERROR ("cpu plugin: fopen (/proc/stat) failed: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		return (-1);
	}

	while (fgets (buf, 1024, fh) != NULL)
	{
		if (strncmp (buf, "cpu", 3))
			continue;
		if ((buf[3] < '0') || (buf[3] > '9'))
			continue;

		numfields = strsplit (buf, fields, 9);
		if (numfields < 5)
			continue;

		cpu = atoi (fields[0] + 3);
		user = atoll (fields[1]);
		nice = atoll (fields[2]);
		syst = atoll (fields[3]);
		idle = atoll (fields[4]);

		submit (cpu, "user", user);
		submit (cpu, "nice", nice);
		submit (cpu, "system", syst);
		submit (cpu, "idle", idle);

		if (numfields >= 8)
		{
			wait = atoll (fields[5]);
			intr = atoll (fields[6]);
			sitr = atoll (fields[7]);

			submit (cpu, "wait", wait);
			submit (cpu, "interrupt", intr);
			submit (cpu, "softirq", sitr);

			if (numfields >= 9)
				submit (cpu, "steal", atoll (fields[8]));
		}
	}

	fclose (fh);
/* #endif defined(KERNEL_LINUX) */

#elif defined(HAVE_LIBKSTAT)
	int cpu;
	counter_t user, syst, idle, wait;
	static cpu_stat_t cs;

	if (kc == NULL)
		return (-1);

	for (cpu = 0; cpu < numcpu; cpu++)
	{
		if (kstat_read (kc, ksp[cpu], &cs) == -1)
			continue; /* error message? */

		idle = (counter_t) cs.cpu_sysinfo.cpu[CPU_IDLE];
		user = (counter_t) cs.cpu_sysinfo.cpu[CPU_USER];
		syst = (counter_t) cs.cpu_sysinfo.cpu[CPU_KERNEL];
		wait = (counter_t) cs.cpu_sysinfo.cpu[CPU_WAIT];

		submit (ksp[cpu]->ks_instance, "user", user);
		submit (ksp[cpu]->ks_instance, "system", syst);
		submit (ksp[cpu]->ks_instance, "idle", idle);
		submit (ksp[cpu]->ks_instance, "wait", wait);
	}
/* #endif defined(HAVE_LIBKSTAT) */

#elif CAN_USE_SYSCTL
	uint64_t cpuinfo[numcpu][CPUSTATES];
	size_t cpuinfo_size;
	int status;
	int i;

	if (numcpu < 1)
	{
		ERROR ("cpu plugin: Could not determine number of "
				"installed CPUs using sysctl(3).");
		return (-1);
	}

	memset (cpuinfo, 0, sizeof (cpuinfo));

#if defined(KERN_CPTIME2)
	if (numcpu > 1) {
		for (i = 0; i < numcpu; i++) {
			int mib[] = {CTL_KERN, KERN_CPTIME2, i};

			cpuinfo_size = sizeof (cpuinfo[0]);

			status = sysctl (mib, STATIC_ARRAY_SIZE (mib),
					cpuinfo[i], &cpuinfo_size, NULL, 0);
			if (status == -1) {
				char errbuf[1024];
				ERROR ("cpu plugin: sysctl failed: %s.",
						sstrerror (errno, errbuf, sizeof (errbuf)));
				return (-1);
			}
		}
	}
	else
#endif /* defined(KERN_CPTIME2) */
	{
		int mib[] = {CTL_KERN, KERN_CPTIME};
		long cpuinfo_tmp[CPUSTATES];

		cpuinfo_size = sizeof(cpuinfo_tmp);

		status = sysctl (mib, STATIC_ARRAY_SIZE (mib),
					&cpuinfo_tmp, &cpuinfo_size, NULL, 0);
		if (status == -1)
		{
			char errbuf[1024];
			ERROR ("cpu plugin: sysctl failed: %s.",
					sstrerror (errno, errbuf, sizeof (errbuf)));
			return (-1);
		}

		for(i = 0; i < CPUSTATES; i++) {
			cpuinfo[0][i] = cpuinfo_tmp[i];
		}
	}

	for (i = 0; i < numcpu; i++) {
		submit (i, "user",      cpuinfo[i][CP_USER]);
		submit (i, "nice",      cpuinfo[i][CP_NICE]);
		submit (i, "system",    cpuinfo[i][CP_SYS]);
		submit (i, "idle",      cpuinfo[i][CP_IDLE]);
		submit (i, "interrupt", cpuinfo[i][CP_INTR]);
	}
/* #endif CAN_USE_SYSCTL */

#elif defined(HAVE_SYSCTLBYNAME)
	long cpuinfo[CPUSTATES];
	size_t cpuinfo_size;

	cpuinfo_size = sizeof (cpuinfo);

	if (sysctlbyname("kern.cp_time", &cpuinfo, &cpuinfo_size, NULL, 0) < 0)
	{
		char errbuf[1024];
		ERROR ("cpu plugin: sysctlbyname failed: %s.",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		return (-1);
	}

	submit (0, "user", cpuinfo[CP_USER]);
	submit (0, "nice", cpuinfo[CP_NICE]);
	submit (0, "system", cpuinfo[CP_SYS]);
	submit (0, "idle", cpuinfo[CP_IDLE]);
	submit (0, "interrupt", cpuinfo[CP_INTR]);
/* #endif HAVE_SYSCTLBYNAME */

#elif defined(HAVE_LIBSTATGRAB)
	sg_cpu_stats *cs;
	cs = sg_get_cpu_stats ();

	if (cs == NULL)
	{
		ERROR ("cpu plugin: sg_get_cpu_stats failed.");
		return (-1);
	}

	submit (0, "idle",   (counter_t) cs->idle);
	submit (0, "nice",   (counter_t) cs->nice);
	submit (0, "swap",   (counter_t) cs->swap);
	submit (0, "system", (counter_t) cs->kernel);
	submit (0, "user",   (counter_t) cs->user);
	submit (0, "wait",   (counter_t) cs->iowait);
#endif /* HAVE_LIBSTATGRAB */

	return (0);
}