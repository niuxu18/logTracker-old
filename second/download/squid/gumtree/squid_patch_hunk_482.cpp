             exit(0);
         case '?':
             opt = optopt;
             /* fall thru to default */
         default:
             fprintf(stderr, "FATAL: Unknown option: -%c\n", opt);
-            usage();
+            usage(argv[0]);
             exit(1);
         }
     }
 }
 
 /* Main program for simple authentication.
