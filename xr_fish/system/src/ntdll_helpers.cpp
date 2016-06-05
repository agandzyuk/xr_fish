#include "ntdll_exports.h"
#include "ntdll_helpers.h"
#include "tools.h"

////////////////////////////////////////////////////////////////////////////
#define STATUS_INVALID_IMAGE_FORMAT ((NTSTATUS)0xC000007B)
#define STATUS_NOT_IMPLEMENTED      ((NTSTATUS)0xC0000002)


////////////////////////////////////////////////////////////////////////////
void Fisher::ThreadCreateStack( SIZE_T StackReserve,
                                SIZE_T StackCommit,
                                PINITIAL_TEB InitialTeb )
{
    PIMAGE_NT_HEADERS Headers;
    ULONG_PTR Stack;
    ULONG Dummy;
    SIZE_T GuardPageSize, GuaranteedStackCommit;

    if ( StackReserve == 0 || StackCommit == 0 )
    {
        Headers = ntdll.RtlCurrentProcessImageHeaders();
        if ( 0 == Headers ) 
            ntdll.NtStatusRaiseException( STATUS_INVALID_IMAGE_FORMAT );
    }

    if ( 0 == StackReserve ) 
        StackReserve = Headers->OptionalHeader.SizeOfStackReserve;

    if ( 0 == StackCommit )
        StackCommit = Headers->OptionalHeader.SizeOfStackCommit;
    else if ( StackCommit >= StackReserve )
        StackReserve = ROUND_UP( StackCommit, 0x100000 );

    StackCommit = ROUND_UP( StackCommit, SystemInformation.dwPageSize );
    StackReserve = ROUND_UP( StackReserve, SystemInformation.dwAllocationGranularity );

    GuaranteedStackCommit = ntdll.NtCurrentTeb()->GuaranteedStackBytes;
    if ( (GuaranteedStackCommit) && (StackCommit < GuaranteedStackCommit) )
        StackCommit = GuaranteedStackCommit;

    if ( StackCommit >= StackReserve )
        StackReserve = ROUND_UP( StackCommit, 0x100000 );

    StackCommit = ROUND_UP( StackCommit, SystemInformation.dwPageSize );
    StackReserve = ROUND_UP( StackReserve, SystemInformation.dwAllocationGranularity );

    /* Reserve memory for the stack */
    Stack = 0;
    ntdll.NtAllocateVirtualMemory( GetCurrentProcess(),
                                   (PVOID*)&Stack,
                                   0,
                                   &StackReserve,
                                   MEM_RESERVE,
                                   PAGE_READWRITE );

    /* Now set up some basic Initial TEB Parameters */
    InitialTeb->AllocatedStackBase = (PVOID)Stack;
    InitialTeb->StackBase = (PVOID)(Stack + StackReserve);
    InitialTeb->PreviousStackBase = NULL;
    InitialTeb->PreviousStackLimit = NULL;

    /* Update the Stack Position */
    Stack += StackReserve - StackCommit;

    /* Allocate memory for the stack */
    try {
        ntdll.NtAllocateVirtualMemory( GetCurrentProcess(),
                                       (PVOID*)&Stack,
                                       0,
                                       &StackCommit,
                                       MEM_COMMIT,
                                       PAGE_READWRITE );
    }
    catch(...)
    {
        GuardPageSize = 0;
        ntdll.NtFreeVirtualMemory( GetCurrentProcess(), (PVOID*)&Stack, &GuardPageSize, MEM_RELEASE );
        throw;
    }

    /* Now set the current Stack Limit */
    InitialTeb->StackLimit = (PVOID)Stack;

    /* Create a guard page */
    GuardPageSize = SystemInformation.dwPageSize;
    ntdll.NtProtectVirtualMemory( GetCurrentProcess(),
                                  (PVOID*)&Stack,
                                  &GuardPageSize,
                                  PAGE_GUARD | PAGE_READWRITE,
                                  &Dummy );

    /* Update the Stack Limit keeping in mind the Guard Page */
    InitialTeb->StackLimit = (PVOID)((ULONG_PTR)InitialTeb->StackLimit + GuardPageSize);
}

////////////////////////////////////////////////////////////////////////////
void Fisher::ThreadFreeStack(PINITIAL_TEB InitialTeb)
{
    SIZE_T Dummy = 0;
    ntdll.NtFreeVirtualMemory( GetCurrentProcess(),
                               &InitialTeb->AllocatedStackBase,
                               &Dummy,
                               MEM_RELEASE );
}

////////////////////////////////////////////////////////////////////////////
void Fisher::ThreadInitializeContext( PCONTEXT Context,
                                      PVOID Parameter,
                                      PVOID StartAddress,
                                      PVOID StackAddress,
                                      ULONG ContextType )
{
    Context->ContextFlags = CONTEXT_FULL | CONTEXT_EXTENDED_REGISTERS;
    Context->EFlags = 0x3000; /* IOPL 3 */

#ifdef _M_IX86
    /* Setup the Initial Win32 Thread Context */
    Context->Eax = (ULONG)StartAddress;
    Context->Ebx = (ULONG)Parameter;
    Context->Esp = (ULONG)StackAddress;
    /* The other registers are undefined */

    /* Setup the Segments */
    Context->SegFs = KGDT_R3_TEB;
    Context->SegEs = KGDT_R3_DATA;
    Context->SegDs = KGDT_R3_DATA;
    Context->SegCs = KGDT_R3_CODE;
    Context->SegSs = KGDT_R3_DATA;
    Context->SegGs = 0;

    /* What kind of context is being created? */
    if ( ContextType == 1 )
    {
        /* For Threads */
        Context->Eip = (ULONG)StartAddress;/*(ULONG)BaseThreadStartupThunk;*/
    }
    else if ( ContextType == 2 )
    {
        /* This is a fiber */
        ntdll.NtStatusRaiseException( STATUS_NOT_IMPLEMENTED );
    }
    else
    {
        /* For first thread in a Process */
        Context->Eip = 0;/*(ULONG)BaseProcessStartThunk;*/
    }

    /* Give it some room for the Parameter */
    Context->Esp -= sizeof(PVOID);

#elif defined(_M_AMD64)
    /* Setup the Initial Win32 Thread Context */
    Context->Rax = (ULONG_PTR)StartAddress;
    Context->Rbx = (ULONG_PTR)Parameter;
    Context->Rsp = (ULONG_PTR)StackAddress;

    /* Setup the Segments */
    Context->SegGs = KGDT64_R3_DATA | RPL_MASK;
    Context->SegEs = KGDT64_R3_DATA | RPL_MASK;
    Context->SegDs = KGDT64_R3_DATA | RPL_MASK;
    Context->SegCs = KGDT64_R3_CODE | RPL_MASK;
    Context->SegSs = KGDT64_R3_DATA | RPL_MASK;
    Context->SegFs = KGDT64_R3_CMTEB | RPL_MASK;

    if ( ContextType == 1 )      /* For Threads */
    {
        Context->Rip = (ULONG_PTR)BaseThreadStartupThunk;
    }
    else if ( ContextType == 2 ) /* For Fibers */
    {
        ntdll.NtStatusRaiseException( STATUS_NOT_IMPLEMENTED );
    }
    else                       /* For first thread in a Process */
    {
        Context->Rip = (ULONG_PTR)BaseProcessStartThunk;
    }

    /* Give it some room for the Parameter */
    Context->Rsp -= sizeof(PVOID);
#endif
}
