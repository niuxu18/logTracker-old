static void
process_options(int argc, char *argv[])
{
    int opt;

    opterr = 0;
    while (-1 != (opt = getopt(argc, argv, "hd"))) {
        switch (opt) {
        case 'd':
            debug_enabled = 1;
            break;
        case 'h':
            usage();
            exit(0);
        default:
            fprintf(stderr, "%s: FATAL: unknown option: -%c. Exiting\n", program_name, opt);
            usage();
            exit(1);
        }
    }
}