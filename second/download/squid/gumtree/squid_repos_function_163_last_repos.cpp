int
main(int argc, char **argv)
{
    Debug::Levels[1] = 8;
    debugs (1,1,"test" << "string");
    debugs (1,9,"dont show this" << "string");
    debugs (1,1,"test" << "string");
    debugs (1,1,"test" << "string");
    if (true)
        debugs(1,9,"this won't compile if the macro is broken.");
    else
        debugs(1, DBG_IMPORTANT,"bar");
    StreamTest aStreamObject;
    StreamTest *streamPointer (&aStreamObject);
    debugs(1, DBG_IMPORTANT,aStreamObject);
    debugs(1, DBG_IMPORTANT,streamPointer->getAnInt() << " " << aStreamObject.getACString());
    return 0;
}