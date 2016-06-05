#ifndef __fisher_task_h__
#define __fisher_task_h__

#include "fisher.h"

namespace Fisher {

/// ThreadState struct enumerator //////////////////////
const unsigned long ThreadStateInit = 0;
enum ThreadState 
{
    Initial    = ThreadStateInit,
    Suspended  = Initial + 1,
    Resumed    = Initial + 2,
    Canceled   = Initial + 4,
    Terminated = Initial + 8,
    Idle       = Initial + 16,
};

/// class Task ////////////////////////////////////////
class Task
{
public:
    Task( );
    virtual ~Task();

    virtual void exec() = 0;

    virtual void suspend();
    virtual void resume();
    virtual void cancel();
    virtual void terminate();

    virtual ThreadState get_state() const;
    virtual long get_priority() const;
    virtual void set_priority(long priority);
    virtual unsigned long get_interval() const;
    virtual void set_interval(unsigned long interval);

    /// ...old....
    bool push_frame();
    void lock();
    int  unlock();

private:
    CRITICAL_SECTION hExclusiveAccess_;
    int stack_state_; /// 0 is free
};

}

#endif  // __fisher_task_h__