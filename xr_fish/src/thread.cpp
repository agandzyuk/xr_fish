#include "thread.h"
#include "exception.h"

using namespace Fisher;


////////////////////////////////////////////////
/// Thread
////////////////////////////////////////////////

Thread::Thread()
    : id_(0)
{
}

Thread::~Thread()
{}

void Thread::set_task(Task& task)
{
    task_.reset( &task );
}

void Thread::resume()
{
    if ( 0 == id_ ) 
        init();

    if ( -1 == ::ResumeThread( (HANDLE)handle_ ) )
        throw Win32Exception( GetLastError() );
}

void Thread::suspend()
{
}

void Thread::cancel()
{
}

void Thread::terminate()
{
}

void Thread::set_idle()
{
}

void Thread::init()
{
    handle_ = (unsigned long)::CreateThread(NULL, 
        4, thread_cb, (void*)this, CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION, &id_);
    if ( 0 == handle_ )
        throw Win32Exception( GetLastError() );

    if ( 0 == ::SetThreadPriority( (HANDLE)handle_, THREAD_PRIORITY_IDLE ) )
        throw Win32Exception( GetLastError() );
}


inline unsigned long __stdcall Thread::thread_cb(void* param)
{
    if ( !param ) 
        return -1;

    Thread* self = reinterpret_cast<Thread*>(param);
    while( true )
    {
        self->run();
    }
}
