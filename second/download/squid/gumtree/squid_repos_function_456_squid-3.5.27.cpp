int
Valid_User(char *USERNAME, char *PASSWORD, const char *SERVER, char *BACKUP, const char *DOMAIN)
{
    const char *supportedDialects[] = {"PC NETWORK PROGRAM 1.0",
                                       "MICROSOFT NETWORKS 1.03",
                                       "MICROSOFT NETWORKS 3.0",
                                       "LANMAN1.0",
                                       "LM1.2X002",
                                       "Samba",
                                       "NT LM 0.12",
                                       "NT LANMAN 1.0",
                                       NULL
                                      };
    SMB_Handle_Type con;

    SMB_Init();
    con = SMB_Connect_Server(NULL, SERVER, DOMAIN);
    if (con == NULL) {
        return (NTV_SERVER_ERROR);
    }
    if (SMB_Negotiate(con, supportedDialects) < 0) {    /* An error */
        SMB_Discon(con, 0);
        return (NTV_PROTOCOL_ERROR);
    }
    if (SMB_Logon_Server(con, USERNAME, PASSWORD, NULL, 0) < 0) {
        SMB_Discon(con, 0);
        return (NTV_LOGON_ERROR);
    }
    SMB_Discon(con, 0);
    return (NTV_NO_ERROR);
}