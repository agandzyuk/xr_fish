#include "ntdll_exports.h"

using namespace Fisher;

////////////////////////////////////////////////////////
#define GetDllExport(module, function) \
    { \
        PVOID key = (PVOID)##function; \
        PVOID val = (PVOID)(type_##function)::GetProcAddress(h,#function); \
        if ( 0 == val ) \
            throw Win32Exception( GetLastError() ); \
        exports_.insert( FuncTableT::value_type(key,val) ); \
    }

////////////////////////////////////////////////////////
NtDllExport Fisher::ntdll;
SYSTEM_INFO Fisher::SystemInformation;

NtDllExport::FuncTableT   NtDllExport::exports_;
PROCESS_BASIC_INFORMATION NtDllExport::ProcessBasicInfo_;


////////////////////////////////////////////////////////
void NtDllExport::LdrReloadExports()
{
    HMODULE h = GetModuleHandleA("ntdll.dll");
    if ( 0 == h ) 
        throw Win32Exception( GetLastError() );

    exports_.clear();

    GetDllExport( h, RtlGetCurrentPeb );
    GetDllExport( h, RtlImageNtHeader );
    GetDllExport( h, RtlAcquirePebLock );
    GetDllExport( h, RtlReleasePebLock );
    
    GetDllExport( h, RtlNtStatusToDosError );
    GetDllExport( h, RtlAnsiStringToUnicodeString );
    GetDllExport( h, RtlUnicodeStringToAnsiString );

    GetDllExport( h, NtCreateSection );
    GetDllExport( h, NtQuerySection );

    GetDllExport( h, NtCreateEvent );
    GetDllExport( h, NtWaitForSingleObject );
    GetDllExport( h, NtSetEvent );
    GetDllExport( h, NtResetEvent );

    GetDllExport( h, NtDelayExecution );
    GetDllExport( h, NtYieldExecution );
    GetDllExport( h, NtQueryInformationProcess );

    GetDllExport( h, NtAllocateVirtualMemory );
    GetDllExport( h, NtFreeVirtualMemory );
    GetDllExport( h, NtProtectVirtualMemory );

    GetDllExport( h, NtCreateThread );
    GetDllExport( h, NtSuspendThread );
    GetDllExport( h, NtAlertResumeThread );

    GetDllExport( h, NtCreateThread );
    GetDllExport( h, NtOpenThread );
    GetDllExport( h, NtClose );

    RtlZeroMemory( &SystemInformation, sizeof(SystemInformation) );
    GetSystemInfo( &SystemInformation );
    
    RtlZeroMemory( &ProcessBasicInfo_, sizeof(ProcessBasicInfo_) );
    NtQueryInformationProcess( ::GetCurrentProcess(), ProcessBasicInformation, &ProcessBasicInfo_ );

    FreeLibrary(h);
}
