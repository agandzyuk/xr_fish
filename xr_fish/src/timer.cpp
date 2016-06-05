#include "timer.h"
#include "task.h"
#include "exception.h"

using namespace Fisher;


////////////////////////////////////////////////
/// Timer
////////////////////////////////////////////////

Timer::Timer()
    : interval_ms_(0), id_(0)
{
}

Timer::Timer(Task* task, unsigned int interval_ms) 
    : task_(task), interval_ms_(interval_ms), id_(0)
{
    init();
}

Timer::~Timer()
{}

void Timer::set_interval(unsigned int millis)
{
    interval_ms_ = millis;
}

void Timer::set_task(Task* task)
{
    task_.reset( task );
}

Task& Timer::get_task() const
{
    return *task_.get();
}

void Timer::start( bool immediately )
{
    if ( 0 == id_ ) 
        init();

    if ( immediately )
        on_tick();

    if ( -1 == ::ResumeThread( hThread_ ) )
        throw Win32Exception( GetLastError() );
}

void Timer::stop( bool immediately )
{
}

void Timer::init()
{
    hThread_ = ::CreateThread(NULL, 
        4, thread_cb, (void*)this, CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION, &id_);
    if ( 0 == hThread_ )
        throw Win32Exception( GetLastError() );

    if ( 0 == ::SetThreadPriority( hThread_, THREAD_PRIORITY_IDLE ) )
        throw Win32Exception( GetLastError() );
}

unsigned long Timer::on_tick()
{
    if ( !task_.get() )
        return 0;

    ExceptionBlock_BEGIN();
    task_->exec();
    ExceptionBlock_END( true, 0 );
    return 0;
}

inline unsigned long CALLBACK Timer::thread_cb(void* param)
{
    if ( !param ) 
        return -1;

    unsigned long thread_return = 0;

    Timer* self = reinterpret_cast<Timer*>(param);
    
    DWORD tm_start = 0;
    while( true )
    {
        Sleep(500);
        tm_start++;
        self->on_tick();
    }

    return thread_return;
}
