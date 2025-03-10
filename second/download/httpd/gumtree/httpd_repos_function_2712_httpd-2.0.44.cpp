static char *apr_error_string(apr_status_t statcode)
{
    switch (statcode) {
    case APR_ENOPOOL:
        return "A new pool could not be created.";
    case APR_EBADDATE:
        return "An invalid date has been provided";
    case APR_EINVALSOCK:
        return "An invalid socket was returned";
    case APR_ENOPROC:
        return "No process was provided and one was required.";
    case APR_ENOTIME:
        return "No time was provided and one was required.";
    case APR_ENODIR:
        return "No directory was provided and one was required.";
    case APR_ENOLOCK:
        return "No lock was provided and one was required.";
    case APR_ENOPOLL:
        return "No poll structure was provided and one was required.";
    case APR_ENOSOCKET:
        return "No socket was provided and one was required.";
    case APR_ENOTHREAD:
        return "No thread was provided and one was required.";
    case APR_ENOTHDKEY:
        return "No thread key structure was provided and one was required.";
    case APR_ENOSHMAVAIL:
        return "No shared memory is currently available";
    case APR_EDSOOPEN:
#if APR_HAS_DSO && defined(HAVE_LIBDL)
        return dlerror();
#else
        return "DSO load failed";
#endif /* HAVE_LIBDL */
    case APR_EBADIP:
        return "The specified IP address is invalid.";
    case APR_EBADMASK:
        return "The specified network mask is invalid.";

    case APR_INCHILD:
        return
	    "Your code just forked, and you are currently executing in the "
	    "child process";
    case APR_INPARENT:
        return
	    "Your code just forked, and you are currently executing in the "
	    "parent process";
    case APR_DETACH:
        return "The specified thread is detached";
    case APR_NOTDETACH:
        return "The specified thread is not detached";
    case APR_CHILD_DONE:
        return "The specified child process is done executing";
    case APR_CHILD_NOTDONE:
        return "The specified child process is not done executing";
    case APR_TIMEUP:
        return "The timeout specified has expired";
    case APR_INCOMPLETE:
        return "Partial results are valid but processing is incomplete";
    case APR_BADCH:
        return "Bad character specified on command line";
    case APR_BADARG:
        return "Missing parameter for the specified command line option";
    case APR_EOF:
        return "End of file found";
    case APR_NOTFOUND:
        return "Could not find specified socket in poll list.";
    case APR_ANONYMOUS:
        return "Shared memory is implemented anonymously";
    case APR_FILEBASED:
        return "Shared memory is implemented using files";
    case APR_KEYBASED:
        return "Shared memory is implemented using a key system";
    case APR_EINIT:
        return
	    "There is no error, this value signifies an initialized "
	    "error code";
    case APR_ENOTIMPL:
        return "This function has not been implemented on this platform";
    case APR_EMISMATCH:
        return "passwords do not match";
    case APR_EABSOLUTE:
        return "The given path is absolute";
    case APR_ERELATIVE:
        return "The given path is relative";
    case APR_EINCOMPLETE:
        return "The given path is incomplete";
    case APR_EABOVEROOT:
        return "The given path was above the root path";
    case APR_EBADPATH:
        return "The given path misformatted or contained invalid characters";
    default:
        return "Error string not specified yet";
    }
}