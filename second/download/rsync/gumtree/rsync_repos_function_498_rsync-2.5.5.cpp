const char * poptGetInvocationName(poptContext con) {
    return con->os->argv[0];
}