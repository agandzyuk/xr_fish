#ifndef __fisher_thread_h__
#define __fisher_thread_h__

#include "fisher.h"
#include "task.h"

#include <memory>

namespace Fisher {

/// class Thread ///////////////////////////////////////
class Thread
{
    friend class Task;

public:
    Thread();
    virtual ~Thread();

    virtual void resume();
    virtual void suspend();
    virtual void cancel();
    virtual void terminate();
    virtual void set_idle();

    ThreadState get_state();
    void set_task(Task& task);

protected:
    virtual void init() = 0;
    virtual void run()  = 0;

protected:
    unsigned long id_;
    unsigned long handle_;
    unsigned char state_;

private:
    inline static unsigned long __stdcall thread_cb(void* param);
    std::auto_ptr<Task> task_;
};

}

#endif  // __fisher_thread_h__