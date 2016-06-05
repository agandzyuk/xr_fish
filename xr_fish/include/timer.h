#ifndef __fisher_timer_h__
#define __fisher_timer_h__

#include "fisher.h"

#include <memory>

namespace Fisher {

class Task;
/// class Timer ///////////////////////////////////////
class Timer
{
    friend class Task;

public:
    Timer();
    Timer(Task* task, unsigned int interval_ms);
    virtual ~Timer();

    virtual void start(bool immediately);
    virtual void stop(bool immediately);
    virtual void set_interval(unsigned int millis);
    virtual void set_task(Task* task);
    virtual Task& get_task() const;

protected:
    virtual void init();
    virtual unsigned long on_tick();

private:
    inline static unsigned long CALLBACK thread_cb(void* param);

private:
    unsigned int        interval_ms_;
    unsigned long       id_;
    HANDLE              hThread_;
    std::auto_ptr<Task> task_;
};

}

#endif  // __fisher_timer_h__