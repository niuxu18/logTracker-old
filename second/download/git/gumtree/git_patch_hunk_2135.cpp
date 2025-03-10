 		if (protocol == PROTO_SSH) {
 			const char *ssh;
 			int putty;
 			char *ssh_host = hostandport;
 			const char *port = NULL;
 			get_host_and_port(&ssh_host, &port);
-			port = get_port_numeric(port);
 
-			ssh = getenv("GIT_SSH_COMMAND");
-			if (ssh) {
-				conn->use_shell = 1;
-				putty = 0;
-			} else {
-				ssh = getenv("GIT_SSH");
-				if (!ssh)
-					ssh = "ssh";
-				putty = !!strcasestr(ssh, "plink");
-			}
+			if (!port)
+				port = get_port(ssh_host);
 
-			argv_array_push(&conn->args, ssh);
-			if (putty && !strcasestr(ssh, "tortoiseplink"))
-				argv_array_push(&conn->args, "-batch");
-			if (port) {
-				/* P is for PuTTY, p is for OpenSSH */
-				argv_array_push(&conn->args, putty ? "-P" : "-p");
-				argv_array_push(&conn->args, port);
+			if (flags & CONNECT_DIAG_URL) {
+				printf("Diag: url=%s\n", url ? url : "NULL");
+				printf("Diag: protocol=%s\n", prot_name(protocol));
+				printf("Diag: userandhost=%s\n", ssh_host ? ssh_host : "NULL");
+				printf("Diag: port=%s\n", port ? port : "NONE");
+				printf("Diag: path=%s\n", path ? path : "NULL");
+
+				free(hostandport);
+				free(path);
+				return NULL;
+			} else {
+				ssh = getenv("GIT_SSH_COMMAND");
+				if (ssh) {
+					conn->use_shell = 1;
+					putty = 0;
+				} else {
+					ssh = getenv("GIT_SSH");
+					if (!ssh)
+						ssh = "ssh";
+					putty = !!strcasestr(ssh, "plink");
+				}
+
+				argv_array_push(&conn->args, ssh);
+				if (putty && !strcasestr(ssh, "tortoiseplink"))
+					argv_array_push(&conn->args, "-batch");
+				if (port) {
+					/* P is for PuTTY, p is for OpenSSH */
+					argv_array_push(&conn->args, putty ? "-P" : "-p");
+					argv_array_push(&conn->args, port);
+				}
+				argv_array_push(&conn->args, ssh_host);
 			}
-			argv_array_push(&conn->args, ssh_host);
 		} else {
 			/* remove repo-local variables from the environment */
 			conn->env = local_repo_env;
 			conn->use_shell = 1;
 		}
 		argv_array_push(&conn->args, cmd.buf);
