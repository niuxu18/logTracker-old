	 */
	return TRUE;

    /* We don't support all this async I/O, Microsoft-specific stuff */
    case HSE_REQ_IO_COMPLETION:
    case HSE_REQ_TRANSMIT_FILE:
	ap_log_rerror(APLOG_MARK, APLOG_WARNING, r,
		    "ISAPI asynchronous I/O not supported: %s", r->filename);
    default:
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }
}
++ apache_1.3.2/src/os/win32/modules.c	1998-07-21 08:06:59.000000000 +0800
