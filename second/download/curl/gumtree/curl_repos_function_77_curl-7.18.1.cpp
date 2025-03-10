int
main(int argc, char *argv[])
{
    URL_FILE *handle;
    FILE *outf;

    int nread;
    char buffer[256];
    const char *url;

    if(argc < 2)
    {
        url="http://192.168.7.3/testfile";/* default to testurl */
    }
    else
    {
        url=argv[1];/* use passed url */
    }

    /* copy from url line by line with fgets */
    outf=fopen("fgets.test","w+");
    if(!outf)
    {
        perror("couldn't open fgets output file\n");
        return 1;
    }

    handle = url_fopen(url, "r");
    if(!handle)
    {
        printf("couldn't url_fopen()\n");
        fclose(outf);
        return 2;
    }

    while(!url_feof(handle))
    {
        url_fgets(buffer,sizeof(buffer),handle);
        fwrite(buffer,1,strlen(buffer),outf);
    }

    url_fclose(handle);

    fclose(outf);


    /* Copy from url with fread */
    outf=fopen("fread.test","w+");
    if(!outf)
    {
        perror("couldn't open fread output file\n");
        return 1;
    }

    handle = url_fopen("testfile", "r");
    if(!handle) {
        printf("couldn't url_fopen()\n");
        fclose(outf);
        return 2;
    }

    do {
        nread = url_fread(buffer, 1,sizeof(buffer), handle);
        fwrite(buffer,1,nread,outf);
    } while(nread);

    url_fclose(handle);

    fclose(outf);


    /* Test rewind */
    outf=fopen("rewind.test","w+");
    if(!outf)
    {
        perror("couldn't open fread output file\n");
        return 1;
    }

    handle = url_fopen("testfile", "r");
    if(!handle) {
        printf("couldn't url_fopen()\n");
        fclose(outf);
        return 2;
    }

        nread = url_fread(buffer, 1,sizeof(buffer), handle);
        fwrite(buffer,1,nread,outf);
        url_rewind(handle);

        buffer[0]='\n';
        fwrite(buffer,1,1,outf);

        nread = url_fread(buffer, 1,sizeof(buffer), handle);
        fwrite(buffer,1,nread,outf);


    url_fclose(handle);

    fclose(outf);


    return 0;/* all done */
}