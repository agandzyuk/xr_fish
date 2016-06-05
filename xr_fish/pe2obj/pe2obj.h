#ifndef  __pe2obj_h__
#define __pe2obj_h__

#include <windows.h>
#include <winternl.h>
#include <tchar.h>

///////////////////////////////////////
struct NtNative
{
    static void reload();
    static __forceinline ULONG RtlNtStatusToDosError( NTSTATUS status );
};
static NtNative ntdll;

#endif // __pe2obj_h__