void
Ipc::Inquirer::handleException(const std::exception& e)
{
    debugs(54, 3, HERE << e.what());
    mustStop("exception");
}