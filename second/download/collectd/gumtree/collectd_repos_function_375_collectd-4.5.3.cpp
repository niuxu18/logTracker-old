static int iptables_config (const char *key, const char *value)
{
	if (strcasecmp (key, "Chain") == 0)
	{
		ip_chain_t temp, *final, **list;
		char *table;
		int   table_len;
		char *chain;
		int   chain_len;

		char *value_copy;
		char *fields[4];
		int   fields_num;
		
		memset (&temp, 0, sizeof (temp));

		value_copy = strdup (value);
		if (value_copy == NULL)
		{
		    char errbuf[1024];
		    ERROR ("strdup failed: %s",
			    sstrerror (errno, errbuf, sizeof (errbuf)));
		    return (1);
		}

		/* Chain <table> <chain> [<comment|num> [name]] */
		fields_num = strsplit (value_copy, fields, 4);
		if (fields_num < 2)
		{
		    free (value_copy);
		    return (1);
		}

		table = fields[0];
		chain = fields[1];

		table_len = strlen (table) + 1;
		if ((unsigned int)table_len > sizeof(temp.table))
		{
			ERROR ("Table `%s' too long.", table);
			free (value_copy);
			return (1);
		}
		sstrncpy (temp.table, table, table_len);

		chain_len = strlen (chain) + 1;
		if ((unsigned int)chain_len > sizeof(temp.chain))
		{
			ERROR ("Chain `%s' too long.", chain);
			free (value_copy);
			return (1);
		}
		sstrncpy (temp.chain, chain, chain_len);

		if (fields_num >= 3)
		{
		    char *comment = fields[2];
		    int   rule = atoi (comment);

		    if (rule)
		    {
			temp.rule.num = rule;
			temp.rule_type = RTYPE_NUM;
		    }
		    else
		    {
			temp.rule.comment = strdup (comment);
			if (temp.rule.comment == NULL)
			{
			    free (value_copy);
			    return (1);
			}
			temp.rule_type = RTYPE_COMMENT;
		    }
		}
		else
		{
		    temp.rule_type = RTYPE_COMMENT_ALL;
		}

		if (fields_num >= 4)
		    sstrncpy (temp.name, fields[3], sizeof (temp.name));

		free (value_copy);
		value_copy = NULL;
		table = NULL;
		chain = NULL;

		list = (ip_chain_t **) realloc (chain_list, (chain_num + 1) * sizeof (ip_chain_t *));
		if (list == NULL)
		{
		    char errbuf[1024];
		    ERROR ("realloc failed: %s",
			    sstrerror (errno, errbuf, sizeof (errbuf)));
		    return (1);
		}

		chain_list = list;
		final = (ip_chain_t *) malloc( sizeof(temp) );
		if (final == NULL) 
		{
		    char errbuf[1024];
		    ERROR ("malloc failed: %s",
			    sstrerror (errno, errbuf, sizeof (errbuf)));
		    return (1);
		}
		memcpy (final, &temp, sizeof (temp));
		chain_list[chain_num] = final;
		chain_num++;

		DEBUG ("Chain #%i: table = %s; chain = %s;", chain_num, final->table, final->chain);
	}
	else 
	{
		return (-1);
	}

	return (0);
}