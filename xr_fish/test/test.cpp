// test.cpp : Defines the entry point for the console application.
//

#include "ntdll_exports.h"

#include "thread.h"
#include "mutex.h"

#include "tools.h"

using namespace Fisher;

////////////////////////////////////////////

CRITICAL_SECTION test_critical_section;
bool test_running = false;
long some_shared_expression = 0;

Mutex* pSharedLock = NULL;

void threads_shared_mutex_use()
{
    if( !pSharedLock )
    {
        pSharedLock = new Mutex();
        printf( "\nMutex owner #%d", GetCurrentThreadId() );
    }

    printf( "\n#%d: tries to locking, ", GetCurrentThreadId() );
    if ( !pSharedLock->TryLock() )
    {
        printf( "\n#%d: but goes on waiting state...", GetCurrentThreadId() );
        pSharedLock->Lock();
        printf( "\n#%d: own locking after wait", GetCurrentThreadId() );
        some_shared_expression++;
        Sleep(100);
        some_shared_expression--;
        if (pSharedLock) {
            printf( "\n#%d: unlock after sleeping...", GetCurrentThreadId() );
            pSharedLock->UnLock();
            printf( "\n#%d: done\n", GetCurrentThreadId() );
        }
    }
    else
    {
        printf( "\n#%d: and owns locking", GetCurrentThreadId() );
        some_shared_expression++;
        Sleep(100);
        some_shared_expression--;
        printf( "\n#%d: unlock after sleeping...", GetCurrentThreadId() );
        pSharedLock->UnLock();
        printf( "\n#%d: done", GetCurrentThreadId() );
    }
}

void threads_shared_mutex_destroy()
{
    if( pSharedLock )
    {
        printf( "\nMutex deleter #%d", GetCurrentThreadId() );
        fflush(stdout);
        delete pSharedLock;
        pSharedLock = NULL;
        printf( "\n#%d: delete done\n", GetCurrentThreadId() );
        fflush(stdout);
    }
}

unsigned long __stdcall cb_test_thread( void* )
{
    try 
    {
        unsigned long retcode = 0;
        while( true ) 
        {
            bool shutdown = false;
            ::EnterCriticalSection( &test_critical_section );
            shutdown = !test_running;
            ::LeaveCriticalSection( &test_critical_section );

            if ( shutdown )
                break;
            threads_shared_mutex_use();
        }
        threads_shared_mutex_destroy();
    }
    catch(const Exception& ex)
    {   
        printf( "\n%s\n", ex.reason() );
        ::MessageBox(0, ex.reason(), _T("test error!"), MB_OK|MB_ICONSTOP); 
        return -1;
    }
    return 0;
}

void test_unicode(const wchar_t* pstr) 
{
}

void test_mutex_shared_use()
{
    test_running = true;
    
    DWORD id = 0, id2 = 0;
    HANDLE h = ::CreateThread(0, 4, cb_test_thread, 
        0, CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION, &id);
    ::ResumeThread(h);

    HANDLE h2 = ::CreateThread(0, 4, cb_test_thread, 
        0, CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION, &id2);
    ::ResumeThread(h2);

    Sleep(5000);
    ::EnterCriticalSection( &test_critical_section );
    test_running = false;
    ::LeaveCriticalSection( &test_critical_section );
    Sleep(100); // wait for mutex delete

    ::CloseHandle( h2 );
    ::CloseHandle( h );
}

int main(int argc, char* argv[])
{
    int ret = -1;
    ::InitializeCriticalSection( &test_critical_section );

    try 
    {
        ntdll.LdrReloadExports();
        //Thread a;
        test_mutex_shared_use();
        ret = 0;
    }
    catch(const Exception& ex)
    {   
        ::MessageBox(0, ex.reason(), _T("test error!"), MB_OK|MB_ICONSTOP); 
    }

    ::MessageBox(0, _T("Test stop!"), _T("Breakpoint!"), MB_OK|MB_ICONINFORMATION); 
    return ret;
}

