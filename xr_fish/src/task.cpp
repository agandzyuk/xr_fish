#include "task.h"
#include "exception.h"

using namespace Fisher;

////////////////////////////////////////////////
/// Task
////////////////////////////////////////////////

Task::Task()
    : stack_state_(0)
{
    InitializeCriticalSection( &hExclusiveAccess_ ); 
}

Task::~Task()
{
    ::DeleteCriticalSection( &hExclusiveAccess_ );
}

void Task::suspend()
{
}

void Task::resume()
{
}

void Task::cancel()
{
}

void Task::terminate()
{
}

long Task::get_priority() const
{
    return 0;
}

void Task::set_priority(long priority)
{
}

unsigned long Task::get_interval() const
{
    return 0;
}

void Task::set_interval(unsigned long interval)
{
}

ThreadState Task::get_state() const
{
    return Initial;
}

bool Task::push_frame()
{
    BOOL allowed = TryEnterCriticalSection( &hExclusiveAccess_ );
    if ( allowed /*&& (allowed = (stack_state_ < 101) )*/ ) {
        ++stack_state_;
        LeaveCriticalSection( &hExclusiveAccess_ );
    }
    return (allowed == TRUE);
}

void Task::lock()
{
    EnterCriticalSection( &hExclusiveAccess_ );
    if( stack_state_ > 1 )
        --stack_state_;
}

int Task::unlock()
{
    int sz = stack_state_;
    if (sz > 1) 
        --stack_state_;
    LeaveCriticalSection( &hExclusiveAccess_ );
    return (sz - 1);
}
