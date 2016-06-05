#include "ntdll_exports.h"
#include "ntdll_helpers.h"

#include "thread.h"
#include "exception.h"

using namespace Fisher;


////////////////////////////////////////////////
/// Thread
////////////////////////////////////////////////

Thread::Thread()
    : id_(0),
    handle_(0),
    state_( Initial )
{
    Init();
}

Thread::~Thread()
{}

void Thread::Resume()
{
    if ( -1 == ::ResumeThread( (HANDLE)handle_ ) )
        throw Win32Exception( GetLastError() );
}

void Thread::Suspend()
{
}

void Thread::Cancel()
{
}

void Thread::Terminate()
{
}

void Thread::SetIdle()
{
}

void Thread::Init()
{
    /*OBJECT_ATTRIBUTES ObjectAttributes;
    CLIENT_ID         ClientId;
    CONTEXT           Context;
    INITIAL_TEB       InitialTeb;

    ThreadCreateStack( 0, 0, &InitialTeb );

    // Create Initial Context
    ThreadInitializeContext( &Context,
                             (PVOID)this,
                             thread_cb,
                             InitialTeb.StackBase,
                             1 );

    // initialize the attributes to zero
    ZeroObjectAttributes( &ObjectAttributes );

    try {
        ntdll.NtCreateThread( (PHANDLE)&handle_,
                               THREAD_ALL_ACCESS,
                               &ObjectAttributes,
                               GetCurrentProcess(),
                               &ClientId,
                               &Context,
                               &InitialTeb,
                               TRUE );
    }
    catch(...)
    {
        ThreadFreeStack( &InitialTeb );
        throw;
    }

    ntdll.NtQueryInformationThread( (HANDLE)handle_,
                                    ThreadBasicInformation,
                                    &ThreadBasicInfo,
                                    sizeof(ThreadBasicInfo),
                                    &ReturnLength );

    ntdll.RtlAllocateActivationContextStack( &ActivationContextStack );

    Teb = ThreadBasicInfo.TebBaseAddress;
    Teb->ActivationContextStackPointer = ActivationContextStack;

    try {
        ntdll.RtlQueryInformationActivationContext( RTL_QUERY_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT,
                                                    NULL,
                                                    0,
                                                    ActivationContextBasicInformation,
                                                    &ActCtxInfo,
                                                    sizeof(ActCtxInfo),
                                                    &ReturnLength );
        if ((ActCtxInfo.hActCtx) && !(ActCtxInfo.dwFlags & 1))
        {
            ntdll.RtlActivateActivationContextEx( RTL_ACTIVATE_ACTIVATION_CONTEXT_EX_FLAG_RELEASE_ON_STACK_DEALLOCATION,
                                                  Teb,
                                                  ActCtxInfo.hActCtx,
                                                  &Cookie );
        }
    }
    catch(...) 
    {
        ntdll.RtlFreeActivationContextStack( Teb->ActivationContextStackPointer );
        throw;
    }

    if ( lpThreadId ) 
        *lpThreadId = HandleToUlong( ClientId.UniqueThread );

    if ( !(dwCreationFlags & CREATE_SUSPENDED) ) 
        ntdll.NtResumeThread( (HANDLE)handle_, &Dummy);
    */
}

unsigned long __stdcall Thread::thread_cb(void* param)
{
    /*if ( !param ) 
        return -1;

    Thread* self = reinterpret_cast<Thread*>(param);
    while( true )
    {
        self->Run();
    }*/
    return -1;
}

void Thread::Run()
{
}