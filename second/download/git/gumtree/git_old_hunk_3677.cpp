				break;
			if (parse_merge_opt(&o, arg + 2))
				die("Unknown option %s", arg);
			continue;
		}
		if (bases_count < ARRAY_SIZE(bases)-1) {
			unsigned char *sha = xmalloc(20);
			if (get_sha1(argv[i], sha))
				die("Could not parse object '%s'", argv[i]);
			bases[bases_count++] = sha;
		}
		else
			warning("Cannot handle more than %d bases. "
				"Ignoring %s.",
				(int)ARRAY_SIZE(bases)-1, argv[i]);
	}
	if (argc - i != 3) /* "--" "<head>" "<remote>" */
		die("Not handling anything other than two heads merge.");

	o.branch1 = argv[++i];
	o.branch2 = argv[++i];

	if (get_sha1(o.branch1, h1))
		die("Could not resolve ref '%s'", o.branch1);
	if (get_sha1(o.branch2, h2))
		die("Could not resolve ref '%s'", o.branch2);

	o.branch1 = better_branch_name(o.branch1);
	o.branch2 = better_branch_name(o.branch2);

	if (o.verbosity >= 3)
		printf("Merging %s with %s\n", o.branch1, o.branch2);

	failed = merge_recursive_generic(&o, h1, h2, bases_count, bases, &result);
	if (failed < 0)
		return 128; /* die() error code */
	return failed;
}
