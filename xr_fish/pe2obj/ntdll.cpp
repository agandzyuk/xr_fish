#include "pe2obj.h"
#include "exception.h"

using namespace std;
using namespace Fisher;

HMODULE s_hntdll( 0 );

//////////////////////////////////////////////////////////////////////////
void NtNative::reload()
{
    // get entry point
    s_hntdll = ::GetModuleHandleA("ntdllu.dll");
    if ( !s_hntdll ) 
    {
        TCHAR msg[512];
        _stprintf_s( msg, 512, _T("ntdll.dll\n%s\n"), Win32Exception( GetLastError() ).reason() );
        throw Exception( msg );
    }
    // free spare reference
    ::FreeLibrary( s_hntdll );
}

//////////////////////////////////////////////////////////////////////////
__forceinline ULONG 
NtNative::RtlNtStatusToDosError( NTSTATUS status )
{
    FARPROC proc = ::GetProcAddress(s_hntdll, "RtlNtStatusToDosError");
    if ( !proc )
        throw Win32Exception( GetLastError() );
    return RtlNtStatusToDosError( status );
}
