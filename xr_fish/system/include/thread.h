#ifndef __fisher_thread_h__
#define __fisher_thread_h__

namespace Fisher {

////////////////////////////////////////////////
class Thread
{
private:
    static const unsigned short ThreadStateInit = 0;
    static unsigned long __stdcall thread_cb(void* param);

public:
    enum State 
    {
        Initial    = ThreadStateInit,
        Suspended  = Initial + 1,
        Resumed    = Initial + 2,
        Canceled   = Initial + 4,
        Terminated = Initial + 8,
        Idle       = Initial + 16,
    };

public:
    Thread();
    virtual ~Thread();

    void Resume();
    void Suspend();
    void Cancel();
    void Terminate();
    void SetIdle();

    State GetState()
    { return state_; }

    long GetID()
    { return id_; }

    unsigned long GetHandle()
    { return handle_; }

protected:
    virtual void Init();
    virtual void Run()/*  = 0*/;

private:
    long id_;
    unsigned long handle_;
    State state_;
};

}

#endif  // __fisher_thread_h__