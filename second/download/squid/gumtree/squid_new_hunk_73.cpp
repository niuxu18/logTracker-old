
protected:
    // XXX: temporary method to replace "delete this" in jobs-in-transition.
    // Will be replaced with calls to mustStop() when transition is complete.
    void deleteThis(const char *aReason);

    // force done() for a reason but continue with the current method
    void mustStop(const char *aReason);

    bool done() const; ///< the job is destroyed in callEnd() when done()

    virtual void start(); ///< called by AsyncStart; do not call directly
    virtual bool doneAll() const; ///< whether positive goal has been reached
    virtual void swanSong() {}; ///< internal cleanup; do not call directly
    virtual const char *status() const; ///< for debugging, starts with space

public:
    bool canBeCalled(AsyncCall &call) const; ///< whether we can be called
    void callStart(AsyncCall &call); ///< called just before the called method
    /// called right after the called job method
    virtual void callEnd(); ///< called right after the called job method
    /// called when the job throws during an async call
    virtual void callException(const std::exception &e);

protected:
    const char *stopReason; ///< reason for forcing done() to be true
    const char *typeName; ///< kid (leaf) class name, for debugging
    AsyncCall::Pointer inCall; ///< the asynchronous call being handled, if any
    const InstanceId<AsyncJob> id; ///< job identifier
};

#endif /* SQUID_ASYNC_JOB_H */
