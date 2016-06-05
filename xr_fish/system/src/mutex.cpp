#include "ntdll_exports.h"
#include "mutex.h"


using namespace Fisher;

////////////////////////////////////////////////
/// Mutex
////////////////////////////////////////////////
Mutex::Mutex() 
    : handle_(0), 
    caller_(0),
    owner_( ::GetCurrentThreadId() ),
    critical_section_( 0 )
{
    ntdll.NtCreateEvent( (PHANDLE)&handle_, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, TRUE );
}

Mutex::~Mutex()
{
    CriticalSectionEnter();
    long deleterId = ::GetCurrentThreadId();
    long lockerId = ::InterlockedCompareExchange( &caller_, 0, deleterId );
    CriticalSectionLeave();
    
    NTSTATUS status = STATUS_WAIT_0;
    if( lockerId ) 
    {
        LARGE_INTEGER timeout;
        status = ntdll.NtDelayExecution( TRUE, ntdll.get_nt_timeout(&timeout, DEADLOCK_WAITER_LIMIT) );
    }

//    NTSTATUS status = ntdll.NtWaitForSingleObject( (HANDLE)handle_, FALSE, 
//                                ntdll.get_nt_timeout(&timeout, DEADLOCK_WAITER_LIMIT) );
//
    if( status != STATUS_WAIT_0 ) 
    {
        char buf[256];
        sprintf_s( buf, 256, 
            "Mutex deadlock detected while destroying!\n"
            "(deleter is #%d, locked by #%d)", 
            deleterId, lockerId );
        throw Exception( buf );
    }

    ntdll.NtClose( (HANDLE)handle_ );
}

void Mutex::Lock()
{
    CriticalSectionEnter();

    long waiterId = ::GetCurrentThreadId();
    long ownerId = ::InterlockedCompareExchange( &caller_, waiterId, 0 );

    while ( ownerId ) 
    {
        CriticalSectionLeave();
        if ( ownerId == waiterId ) // repeating Lock
            break;

        if( waiterId & ownerId & ~owner_ )
        {
            char buf[256];
            sprintf_s( buf, 256, 
                "Mutex Lock using error!\n"
                "At once waiter or locker must have the ownership\n"
                "(ownerId #%d, waiter is #%d, owned by #%d)", 
                owner_, waiterId, ownerId );
            throw Exception( buf );
        }


        LARGE_INTEGER timeout;
        NTSTATUS status = ntdll.NtDelayExecution( TRUE, ntdll.get_nt_timeout(&timeout, DEADLOCK_WAITER_LIMIT) );
        if( status != STATUS_WAIT_0 ) 
        {
            CriticalSectionLeave();

            char buf[256];
            sprintf_s( buf, 256, 
                "Mutex deadlock detected!\n"
                "(waiter is #%d, owned by #%d)", 
                waiterId, ownerId );
            throw Exception( buf );
        }

        printf("\n#%d: Mutex::Lock() STATUS_WAIT_0", waiterId );

        CriticalSectionEnter();
        ownerId = ::InterlockedCompareExchange( &caller_, waiterId, 0 );
    }

    ntdll.NtResetEvent( (HANDLE)handle_, NULL );
    CriticalSectionLeave();
}

void Mutex::UnLock()
{
    CriticalSectionEnter();

    long unlockerId = ::GetCurrentThreadId();
    long ownerId  = ::InterlockedCompareExchange( &caller_, 0, unlockerId );

    if( ownerId & ~unlockerId ) 
    {
        ::InterlockedExchange( &caller_, ownerId );
        CriticalSectionLeave();

        char buf[256];
        sprintf_s( buf, 256, 
            "Mutex UnLock using error!\n"
            "Unlock made outside of waiter's context\n"
            "(unlocker is #%d, owned by #%d)", 
            unlockerId, ownerId );
        throw Exception( buf );
    }
    
    if ( ownerId ) 
    {
        ULONG suspends_count = 0;
        HANDLE hThreadSuspended = ::OpenThread( THREAD_SUSPEND_RESUME, FALSE, ownerId );
        if ( 0 == hThreadSuspended )
            throw Win32Exception( ::GetLastError() );
        ntdll.NtAlertResumeThread( hThreadSuspended, &suspends_count );
        suspends_count = suspends_count;
    }

//    ntdll.NtSetEvent( (HANDLE)handle_, NULL );
    CriticalSectionLeave();
}

bool Mutex::TryLock()
{
    CriticalSectionEnter();

    long waiterId = ::GetCurrentThreadId();
    long lockerId = ::InterlockedCompareExchange( &caller_, waiterId, 0 );

    if( lockerId )
    {
        if( lockerId == waiterId ) // repeating TryLock
            lockerId = 0;
        else if ( lockerId != waiterId && ( lockerId & waiterId & ~owner_ ) ) 
        {
            ::InterlockedExchange( &caller_, lockerId );
            CriticalSectionLeave();

            char buf[256];
            sprintf_s( buf, 256, 
                "Mutex TryLock using error!\n"
                "At once waiter or locker must have the ownership\n"
                "(ownerId #%d, waiter is #%d, locker is #%d)", 
                owner_, waiterId, lockerId );
            throw Exception( buf );
        }
    }
    else // lockerId == caller_
        ntdll.NtResetEvent( (HANDLE)handle_, NULL );
        
    CriticalSectionLeave();
    return (0 == lockerId);
}

inline void Mutex::CriticalSectionEnter()
{
    long free = 1;
    for( ; free == 1 ; )
    {
        free = ::InterlockedCompareExchange( &critical_section_, 1, 0 );
    }
}

inline void Mutex::CriticalSectionLeave()
{
    ::InterlockedExchange( &critical_section_, 0 );
}