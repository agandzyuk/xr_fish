#ifndef __fisher_ntdll_exports_h__
#define __fisher_ntdll_exports_h__

#include "ntdll_types.h"
#include "exception.h"

#include <map>

////////////////////////////////////////////////////////
namespace Fisher 
{ 

////////////////////////////////////////////////////////
#define NtLtrType(tl) tl

////////////////////////////////////////////////////////
#define InvoceNtFuncT( Name ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    return exec()
#define InvoceNtFuncT1( Name, arg1 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    return exec( arg1 )
#define InvoceNtFuncT2( Name, arg1, arg2 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    return exec( arg1, arg2 )
#define InvoceNtFuncT3( Name, arg1, arg2, arg3 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    return exec( arg1, arg2, arg3 )

#define InvoceNtFuncV( Name ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec() )
#define InvoceNtFuncV1( Name, arg1 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec( arg1 ) )
#define InvoceNtFuncV2( Name, arg1, arg2 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec( arg1, arg2 ) )
#define InvoceNtFuncV3( Name, arg1, arg2, arg3 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec( arg1, arg2, arg3 ) )
#define InvoceNtFuncV4( Name, arg1, arg2, arg3, arg4 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec( arg1, arg2, arg3, arg4 ) )
#define InvoceNtFuncV5( Name, arg1, arg2, arg3, arg4, arg5 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec( arg1, arg2, arg3, arg4, arg5 ) )
#define InvoceNtFuncV6( Name, arg1, arg2, arg3, arg4, arg5, arg6 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec( arg1, arg2, arg3, arg4, arg5, arg6 ) )
#define InvoceNtFuncV7( Name, arg1, arg2, arg3, arg4, arg5, arg6, arg7 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec( arg1, arg2, arg3, arg4, arg5, arg6, arg7 ) )
#define InvoceNtFuncV8( Name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 ) \
    type_##Name exec = (type_##Name)exports_[(PVOID)##Name]; \
    NtStatusRaiseException( exec( arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 ) )

////////////////////////////////////////////////////////
#define DeclareNtFuncT( RetType, Name ) \
    typedef RetType(__stdcall * type_##Name)();
#define DeclareNtFuncT1( RetType, Name, arg ) \
    typedef RetType(__stdcall * type_##Name)(arg);
#define DeclareNtFuncT2( RetType, Name, arg1, arg2 ) \
    typedef RetType(__stdcall * type_##Name)(arg1, arg2);
#define DeclareNtFuncT3( RetType, Name, arg1, arg2, arg3 ) \
    typedef RetType(__stdcall * type_##Name)(arg1, arg2, arg3);

#define DeclareNtFuncV( Name ) \
    typedef NTSTATUS(__stdcall * type_##Name)(VOID);
#define DeclareNtFuncV1( Name, arg1 ) \
    typedef NTSTATUS(__stdcall * type_##Name)(arg1);
#define DeclareNtFuncV2( Name, arg1, arg2 ) \
    typedef NTSTATUS(__stdcall * type_##Name)(arg1, arg2);
#define DeclareNtFuncV3( Name, arg1, arg2, arg3 ) \
    typedef NTSTATUS(__stdcall * type_##Name)(arg1, arg2, arg3);
#define DeclareNtFuncV4( Name, arg1, arg2, arg3, arg4 ) \
    typedef NTSTATUS(__stdcall * type_##Name)(arg1, arg2, arg3, arg4);
#define DeclareNtFuncV5( Name, arg1, arg2, arg3, arg4, arg5 ) \
    typedef NTSTATUS(__stdcall * type_##Name)(arg1, arg2, arg3, arg4, arg5);
#define DeclareNtFuncV6( Name, arg1, arg2, arg3, arg4, arg5, arg6 ) \
    typedef NTSTATUS(__stdcall * type_##Name)(arg1, arg2, arg3, arg4, arg5, arg6);
#define DeclareNtFuncV7( Name, arg1, arg2, arg3, arg4, arg5, arg6, arg7 ) \
    typedef NTSTATUS(__stdcall * type_##Name)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
#define DeclareNtFuncV8( Name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 ) \
    typedef NTSTATUS(__stdcall * type_##Name)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);

////////////////////////////////////////////////////////
struct NtDllExport
{
    ////////////////////////////////////////////////////////
    DeclareNtFuncT( PPEB_ntexport, RtlGetCurrentPeb );
    static PPEB_ntexport
    RtlGetCurrentPeb( void )
    {
        InvoceNtFuncT( RtlGetCurrentPeb );
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncT1( PIMAGE_NT_HEADERS, RtlImageNtHeader, PVOID );
    static PIMAGE_NT_HEADERS
    RtlImageNtHeader( PVOID BaseAddress )
    {
        InvoceNtFuncT1( RtlImageNtHeader, BaseAddress );
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV(RtlAcquirePebLock);
    static void
    RtlAcquirePebLock()
    {
        InvoceNtFuncV(RtlAcquirePebLock);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV(RtlReleasePebLock);
    static void
    RtlReleasePebLock()
    {
        InvoceNtFuncV(RtlReleasePebLock);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncT1( ULONG, RtlNtStatusToDosError, NTSTATUS );
    static ULONG
    RtlNtStatusToDosError( NTSTATUS Status )
    {
        InvoceNtFuncT1( RtlNtStatusToDosError, Status );
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV3(RtlAnsiStringToUnicodeString, PUNICODE_STRING, PCANSI_STRING, BOOLEAN);
    static void
    RtlAnsiStringToUnicodeString(
        PUNICODE_STRING DestinationString,
        PCANSI_STRING SourceString,
        BOOLEAN AllocateDestinationString
    )
    {
        InvoceNtFuncV3( RtlAnsiStringToUnicodeString, DestinationString, SourceString, AllocateDestinationString );
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV3(RtlUnicodeStringToAnsiString, PANSI_STRING, PCUNICODE_STRING, BOOLEAN);
    static void
    RtlUnicodeStringToAnsiString(
        PANSI_STRING DestinationString,
        PCUNICODE_STRING SourceString,
        BOOLEAN AllocateDestinationString )
    {
        InvoceNtFuncV3( RtlUnicodeStringToAnsiString, DestinationString, SourceString, AllocateDestinationString );
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV5( NtQuerySection, HANDLE, SECTION_INFORMATION_CLASS, PVOID, SIZE_T, PSIZE_T );
    static void 
    NtQuerySection(
        HANDLE SectionHandle,
        SECTION_INFORMATION_CLASS SectionInformationClass,
        PVOID SectionInformation,
        SIZE_T Length,
        PSIZE_T ResultLength )
    { 
        InvoceNtFuncV5( NtQuerySection, SectionHandle, SectionInformationClass, SectionInformation, Length, ResultLength ); 
    } 

    ////////////////////////////////////////////////////////
    DeclareNtFuncV7( NtCreateSection, PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
    static void 
    NtCreateSection(
        PHANDLE SectionHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        PLARGE_INTEGER MaximumSize,
        ULONG SectionPageProtection,
        ULONG AllocationAttributes,
        HANDLE FileHandle )
    { 
        InvoceNtFuncV7( NtCreateSection, SectionHandle, DesiredAccess, ObjectAttributes, MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle ); 
    } 

    ////////////////////////////////////////////////////////
    DeclareNtFuncV5(NtCreateEvent, PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, EVENT_TYPE, BOOLEAN);
    static void 
    NtCreateEvent( 
        PHANDLE EventHandle, 
        ACCESS_MASK DesiredAccess, 
        POBJECT_ATTRIBUTES ObjectAttributes, 
        EVENT_TYPE EventType, 
        BOOLEAN InitialState )  
    { 
        InvoceNtFuncV5( NtCreateEvent, EventHandle, DesiredAccess, ObjectAttributes, EventType, InitialState ); 
    } 

    ////////////////////////////////////////////////////////
    DeclareNtFuncT3(NTSTATUS, NtWaitForSingleObject, HANDLE, BOOLEAN, PLARGE_INTEGER);
    static NTSTATUS 
    NtWaitForSingleObject (
        HANDLE Object,
        BOOLEAN Alertable,
        PLARGE_INTEGER Time )  
    { 
        InvoceNtFuncT3( NtWaitForSingleObject, Object, Alertable, Time ); 
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncT2(NTSTATUS, NtSetEvent, HANDLE, PLONG);
    static NTSTATUS 
    NtSetEvent(
        HANDLE EventHandle,
        PLONG PreviousState )
    { 
        InvoceNtFuncT2( NtSetEvent, EventHandle, PreviousState ); 
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncT2(NTSTATUS, NtResetEvent, HANDLE, PLONG);
    static NTSTATUS 
    NtResetEvent(
        HANDLE EventHandle,
        PLONG NumberOfWaitingThreads )
    { 
        InvoceNtFuncT2( NtResetEvent, EventHandle, NumberOfWaitingThreads ); 
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncT2(NTSTATUS, NtDelayExecution, BOOLEAN, const LARGE_INTEGER*);
    static NTSTATUS 
    NtDelayExecution(
        BOOLEAN Alertable,
        LARGE_INTEGER *Interval )
    { 
        InvoceNtFuncT2( NtDelayExecution, Alertable, Interval ); 
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV(NtYieldExecution);
    static void
    NtYieldExecution()
    {
        InvoceNtFuncV(NtYieldExecution);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV5(NtQueryInformationProcess, HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
    static void
    NtQueryInformationProcess(
        HANDLE ProcessHandle,
        PROCESSINFOCLASS ProcessInformationClass,
        PPROCESS_BASIC_INFORMATION ProcessInformation )
    {
        ULONG Dummy;
        InvoceNtFuncV5(NtQueryInformationProcess, ProcessHandle, ProcessInformationClass, ProcessInformation, sizeof(*ProcessInformation), &Dummy);
    }


    ////////////////////////////////////////////////////////
    DeclareNtFuncV6(NtAllocateVirtualMemory, HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
    static void
    NtAllocateVirtualMemory(
        HANDLE    ProcessHandle,
        PVOID*    BaseAddress,
        ULONG_PTR ZeroBits,
        PSIZE_T   RegionSize,
        ULONG     AllocationType,
        ULONG     Protect )
    {
        InvoceNtFuncV6(NtAllocateVirtualMemory, ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV4(NtFreeVirtualMemory, HANDLE, PVOID*, PSIZE_T, ULONG);
    static void
    NtFreeVirtualMemory(
        HANDLE  ProcessHandle,
        PVOID*  BaseAddress,
        PSIZE_T RegionSize,
        ULONG   FreeType )
    {
        InvoceNtFuncV4(NtFreeVirtualMemory, ProcessHandle, BaseAddress, RegionSize, FreeType);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV5(NtProtectVirtualMemory, HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
    static void
    NtProtectVirtualMemory(
        HANDLE  ProcessHandle,
        PVOID*  BaseAddress,
        PSIZE_T NumberOfBytesToProtect,
        ULONG   NewAccessProtection,
        PULONG  OldAccessProtection )
    {
        InvoceNtFuncV5(NtProtectVirtualMemory, ProcessHandle, BaseAddress, NumberOfBytesToProtect, NewAccessProtection, OldAccessProtection);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV8(NtCreateThread, PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, HANDLE, PCLIENT_ID, PCONTEXT, PINITIAL_TEB, BOOLEAN);
    static void
    NtCreateThread(
        PHANDLE ThreadHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        HANDLE ProcessHandle,
        PCLIENT_ID ClientId,
        PCONTEXT ThreadContext,
        PINITIAL_TEB UserStack,
        BOOLEAN CreateSuspended )
    {
        InvoceNtFuncV8(NtCreateThread, ThreadHandle, DesiredAccess, ObjectAttributes, ProcessHandle, ClientId, ThreadContext, UserStack, CreateSuspended);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV2(NtSuspendThread, HANDLE, PULONG);
    static void
    NtSuspendThread( 
        HANDLE ThreadHandle, 
        PULONG PreviousSuspendCount )
    {
        InvoceNtFuncV2(NtSuspendThread, ThreadHandle, PreviousSuspendCount);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV2(NtAlertResumeThread, HANDLE, PULONG);
    static void
    NtAlertResumeThread( 
        HANDLE ThreadHandle, 
        PULONG SuspendCount )
    {
        InvoceNtFuncV2(NtAlertResumeThread, ThreadHandle, SuspendCount);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV4(NtOpenThread, PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
    static void
    NtOpenThread( 
        PHANDLE ThreadHandle, 
        ACCESS_MASK AccessMask,
        POBJECT_ATTRIBUTES ObjectAttributes,
        PCLIENT_ID ClientId )
    {
        InvoceNtFuncV4(NtOpenThread, ThreadHandle, AccessMask, ObjectAttributes, ClientId);
    }

    ////////////////////////////////////////////////////////
    DeclareNtFuncV1(NtClose, HANDLE);
    static void
    NtClose( HANDLE Handle )
    {
        InvoceNtFuncV1(NtClose, Handle);
    }

    ////////////////////////////////////////////////////////
    static void LdrReloadExports();

    ////////////////////////////////////////////////////////
    static void 
    NtStatusRaiseException( NTSTATUS status ) 
    {
        if ( status ) {
            unsigned long syscode = RtlNtStatusToDosError( status );
            if ( syscode == 0xEA || syscode == 0x13D )
                ::RaiseException( status, 0, 0, 0 );
            else if ( syscode ) 
                throw Win32Exception( syscode );
            throw UnexpectedException();
        }
    }

    ////////////////////////////////////////////////////////
    static PTEB_ntexport
    NtCurrentTeb( void )
    {
        return (PTEB_ntexport)::NtCurrentTeb();
    }

    ////////////////////////////////////////////////////////
    static PIMAGE_NT_HEADERS
    RtlCurrentProcessImageHeaders( void )
    {
        PIMAGE_NT_HEADERS hdrs = 0;
        PVOID ImageBaseAddress = RtlGetCurrentPeb()->ImageBaseAddress;
        if( ImageBaseAddress )
            hdrs = RtlImageNtHeader( ImageBaseAddress );
        return hdrs;
    }

    static PPROCESS_BASIC_INFORMATION
    GetCurrentProcessBasicInformation( void )
    {
        return &ProcessBasicInfo_;
    }

    ////////////////////////////////////////////////////////
    static PLARGE_INTEGER 
    get_nt_timeout( PLARGE_INTEGER pTime, ULONG timeout )
    {
        if (timeout == INFINITE) return NULL;
        pTime->QuadPart = (ULONGLONG)timeout * -10000;
        return pTime;
    }

private:
    ////////////////////////////////////////////////////////
    typedef std::map<PVOID,PVOID>  FuncTableT;
    static FuncTableT exports_;
    static PROCESS_BASIC_INFORMATION ProcessBasicInfo_;
};

////////////////////////////////////////////////////////////
extern NtDllExport ntdll;
extern SYSTEM_INFO SystemInformation;

}
#endif // __fisher_ntdll_exports_h__
