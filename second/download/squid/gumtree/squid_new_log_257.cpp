storeAppendPrintf(sentry, "%5d-%5d: %9.0f %2.0f%%\n",
                          i ? (1 << (i - 1)) + 1 : 1,
                          1 << i,
                          stats.http_read_hist[i],
                          Math::doublePercent(stats.http_read_hist[i], stats.http_reads));