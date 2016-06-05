#ifndef __fisher_ntdll_types_h__
#define __fisher_ntdll_types_h__

#include <windows.h>
#include <winternl.h>

#ifdef _M_IX86
    #include "i386/ketypes.h"
    #include "i386/mmtypes.h"
#elif defined(_M_AMD64)
    #include "amd64/ketypes.h"
    #include "amd64/mmtypes.h"
#endif

///////////////////////////////////////////////////////
typedef enum _EVENT_TYPE
{
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE;

///////////////////////////////////////////////////////
typedef struct _CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

///////////////////////////////////////////////////////
typedef enum _SECTION_INFORMATION_CLASS
{
    SectionBasicInformation,
    SectionImageInformation,
} SECTION_INFORMATION_CLASS;

///////////////////////////////////////////////////////
typedef struct _SECTION_BASIC_INFORMATION {
    ULONG BaseAddress;
    ULONG Attributes;
    LARGE_INTEGER Size;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

///////////////////////////////////////////////////////
typedef struct _SECTION_IMAGE_INFORMATION {
    PVOID EntryPoint;
    ULONG StackZeroBits;
    ULONG StackReserved;
    ULONG StackCommit;
    ULONG ImageSubsystem;
    WORD SubsystemVersionLow;
    WORD SubsystemVersionHigh;
    ULONG Unknown1;
    ULONG ImageCharacteristics;
    ULONG ImageMachineType;
    ULONG Unknown2[3];
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

///////////////////////////////////////////////////////
typedef struct _PEB_FREE_BLOCK
{
    struct _PEB_FREE_BLOCK* Next;
    ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

///////////////////////////////////////////////////////
#if (defined(_WIN64) && !defined(EXPLICIT_32BIT)) || defined(EXPLICIT_64BIT)
  #define GDI_HANDLE_BUFFER_SIZE 60
#else
  #define GDI_HANDLE_BUFFER_SIZE 34
#endif

///////////////////////////////////////////////////////
typedef struct _INITIAL_TEB
{
    PVOID PreviousStackBase;
    PVOID PreviousStackLimit;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID AllocatedStackBase;
} INITIAL_TEB, *PINITIAL_TEB;

///////////////////////////////////////////////////////
typedef struct _INITIAL_PEB
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union
    {
        BOOLEAN BitField;
#if (NTDDI_VERSION >= NTDDI_WS03)
        struct
        {
            BOOLEAN ImageUsesLargePages:1;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
            BOOLEAN IsProtectedProcess:1;
            BOOLEAN IsLegacyProcess:1;
            BOOLEAN SpareBits:5;
#else
            BOOLEAN SpareBits:7;
#endif
        };
#else
        BOOLEAN SpareBool;
#endif
    };
    HANDLE Mutant;
} INITIAL_PEB, *PINITIAL_PEB;

///////////////////////////////////////////////////////
#if(WINVER < 0x0600)
typedef struct _PEB_LDR_DATA
{
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
#if (NTDDI_VERSION >= NTDDI_WIN7)
    UCHAR ShutdownInProgress;
    PVOID ShutdownThreadId;
#endif
} PEB_LDR_DATA, *PPEB_LDR_DATA;
#endif

///////////////////////////////////////////////////////
typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

///////////////////////////////////////////////////////
typedef struct _RTLP_CURDIR_REF
{
    LONG RefCount;
    HANDLE Handle;
} RTLP_CURDIR_REF, *PRTLP_CURDIR_REF;

///////////////////////////////////////////////////////
typedef struct _RTL_RELATIVE_NAME_U
{
    UNICODE_STRING RelativeName;
    HANDLE ContainingDirectory;
    PRTLP_CURDIR_REF CurDirRef;
} RTL_RELATIVE_NAME_U, *PRTL_RELATIVE_NAME_U;

///////////////////////////////////////////////////////
typedef struct _RTL_DRIVE_LETTER_CURDIR
{
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    UNICODE_STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

///////////////////////////////////////////////////////
#if(WINVER < 0x0600)
typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    ULONG MaximumLength;
    ULONG Length;
    ULONG Flags;
    ULONG DebugFlags;
    HANDLE ConsoleHandle;
    ULONG ConsoleFlags;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;
    CURDIR CurrentDirectory;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PWSTR Environment;
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectories[512];
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    SIZE_T EnvironmentSize;
#endif
#if (NTDDI_VERSION >= NTDDI_WIN7)
    SIZE_T EnvironmentVersion;
#endif
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;
#endif

///////////////////////////////////////////////////////
typedef struct _ACTIVATION_CONTEXT_DATA
{
    ULONG Magic;
    ULONG HeaderSize;
    ULONG FormatVersion;
    ULONG TotalSize;
    ULONG DefaultTocOffset;
    ULONG ExtendedTocOffset;
    ULONG AssemblyRosterOffset;
    ULONG Flags;
} ACTIVATION_CONTEXT_DATA, *PACTIVATION_CONTEXT_DATA;

///////////////////////////////////////////////////////
typedef NTSTATUS (NTAPI *PPOST_PROCESS_INIT_ROUTINE)(VOID);

///////////////////////////////////////////////////////
typedef struct _PEB_ntexport /* : public _PEB*/
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
#if (NTDDI_VERSION >= NTDDI_WS03)
    union
    {
        BOOLEAN BitField;
        struct
        {
            BOOLEAN ImageUsesLargePages:1;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
            BOOLEAN IsProtectedProcess:1;
            BOOLEAN IsLegacyProcess:1;
            BOOLEAN IsImageDynamicallyRelocated:1;
            BOOLEAN SkipPatchingUser32Forwarders:1;
            BOOLEAN SpareBits:3;
#else
            BOOLEAN SpareBits:7;
#endif
        };
    };
#else
    BOOLEAN SpareBool;
#endif
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PRTL_CRITICAL_SECTION FastPebLock;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID AltThunkSListPtr;
    PVOID IFEOKey;
    union
    {
        ULONG CrossProcessFlags;
        struct
        {
            ULONG ProcessInJob:1;
            ULONG ProcessInitializing:1;
            ULONG ProcessUsingVEH:1;
            ULONG ProcessUsingVCH:1;
            ULONG ReservedBits0:28;
        };
    };
    union
    {
        PVOID KernelCallbackTable;
        PVOID UserSharedInfoPtr;
    };
#elif (NTDDI_VERSION >= NTDDI_WS03)
    PVOID AltThunkSListPtr;
    PVOID SparePtr2;
    ULONG EnvironmentUpdateCount;
    PVOID KernelCallbackTable;
#else
    PPEBLOCKROUTINE FastPebLockRoutine;
    PPEBLOCKROUTINE FastPebUnlockRoutine;
    ULONG EnvironmentUpdateCount;
    PVOID KernelCallbackTable;
#endif
    ULONG SystemReserved[1];
    ULONG SpareUlong; // AtlThunkSListPtr32
    PPEB_FREE_BLOCK FreeList;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];
    PVOID ReadOnlySharedMemoryBase;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID HotpatchInformation;
#else
    PVOID ReadOnlySharedMemoryHeap;
#endif
    PVOID* ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;
    LARGE_INTEGER CriticalSectionTimeout;
    ULONG_PTR HeapSegmentReserve;
    ULONG_PTR HeapSegmentCommit;
    ULONG_PTR HeapDeCommitTotalFreeThreshold;
    ULONG_PTR HeapDeCommitFreeBlockThreshold;
    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID* ProcessHeaps;
    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    ULONG GdiDCAttributeList;
    PRTL_CRITICAL_SECTION LoaderLock;
    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG_PTR ImageProcessAffinityMask;
    ULONG GdiHandleBuffer[GDI_HANDLE_BUFFER_SIZE];
    PPOST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    PVOID TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];
    ULONG SessionId;
#if (NTDDI_VERSION >= NTDDI_WINXP)
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PVOID pShimData;
    PVOID AppCompatInfo;
    UNICODE_STRING CSDVersion;
    PACTIVATION_CONTEXT_DATA ActivationContextData;
    PVOID ProcessAssemblyStorageMap;
    PACTIVATION_CONTEXT_DATA SystemDefaultActivationContextData;
    PVOID SystemAssemblyStorageMap;
    ULONG_PTR MinimumStackCommit;
#endif
#if (NTDDI_VERSION >= NTDDI_WS03)
    PVOID* FlsCallback;
    LIST_ENTRY FlsListHead;
    PVOID FlsBitmap;
    ULONG FlsBitmapBits[4]; // [FLS_MAXIMUM_AVAILABLE/(sizeof(ULONG)*8)];
    ULONG FlsHighIndex;
#endif
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID WerRegistrationData;
    PVOID WerShipAssertPtr;
#endif
} PEB_ntexport, *PPEB_ntexport;

///////////////////////////////////////////////////////
typedef struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME
{
    struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME *Previous;
    PVOID ActivationContext;
    ULONG Flags;
} RTL_ACTIVATION_CONTEXT_STACK_FRAME, *PRTL_ACTIVATION_CONTEXT_STACK_FRAME;

///////////////////////////////////////////////////////
#if (NTDDI_VERSION >= NTDDI_WS03)
typedef struct _ACTIVATION_CONTEXT_STACK
{
    PRTL_ACTIVATION_CONTEXT_STACK_FRAME ActiveFrame;
    LIST_ENTRY FrameListCache;
    ULONG Flags;
    ULONG NextCookieSequenceNumber;
    ULONG StackId;
} ACTIVATION_CONTEXT_STACK, *PACTIVATION_CONTEXT_STACK;
#else
typedef struct _ACTIVATION_CONTEXT_STACK
{
    ULONG Flags;
    ULONG NextCookieSequenceNumber;
    PVOID ActiveFrame;
    LIST_ENTRY FrameListCache;
} ACTIVATION_CONTEXT_STACK, *PACTIVATION_CONTEXT_STACK;
#endif

///////////////////////////////////////////////////////
#ifndef GDI_BATCH_BUFFER_SIZE
#define GDI_BATCH_BUFFER_SIZE 0x136
#endif

typedef struct _GDI_TEB_BATCH
{
    ULONG  Offset;
    HANDLE HDC;
    ULONG  Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH, *PGDI_TEB_BATCH;

///////////////////////////////////////////////////////
typedef struct _TEB_ACTIVE_FRAME_CONTEXT
{
    ULONG Flags;
    LPSTR FrameName;
} TEB_ACTIVE_FRAME_CONTEXT, *PTEB_ACTIVE_FRAME_CONTEXT;

///////////////////////////////////////////////////////
typedef struct _TEB_ACTIVE_FRAME
{
    ULONG Flags;
    struct _TEB_ACTIVE_FRAME *Previous;
    PTEB_ACTIVE_FRAME_CONTEXT Context;
} TEB_ACTIVE_FRAME, *PTEB_ACTIVE_FRAME;

///////////////////////////////////////////////////////
typedef struct _TEB_ntexport /* : public _TEB */
{
    NT_TIB      NtTib;
    PVOID       EnvironmentPointer;
    CLIENT_ID   ClientId;
    PVOID       ActiveRpcHandle;
    PVOID       ThreadLocalStoragePointer;
    PPEB        ProcessEnvironmentBlock;
    ULONG       LastErrorValue;
    ULONG       CountOfOwnedCriticalSections;
    PVOID       CsrClientThread;
    PVOID       Win32ThreadInfo;
    ULONG       User32Reserved[26];
    ULONG       UserReserved[5];
    PVOID       WOW32Reserved;
    LCID        CurrentLocale;
    ULONG       FpSoftwareStatusRegister;
    PVOID       SystemReserved1[54];
    LONG        ExceptionCode;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PACTIVATION_CONTEXT_STACK ActivationContextStackPointer;
    UCHAR       SpareBytes1[0x30 - 3 * sizeof(PVOID)];
    ULONG       TxFsContext;
#elif (NTDDI_VERSION >= NTDDI_WS03)
    PACTIVATION_CONTEXT_STACK ActivationContextStackPointer;
    UCHAR       SpareBytes1[0x34 - 3 * sizeof(PTR(PVOID))];
#else
    ACTIVATION_CONTEXT_STACK ActivationContextStack;
    UCHAR       SpareBytes1[24];
#endif
    GDI_TEB_BATCH  GdiTebBatch;
    CLIENT_ID      RealClientId;
    PVOID          GdiCachedProcessHandle;
    ULONG          GdiClientPID;
    ULONG          GdiClientTID;
    PVOID          GdiThreadLocalInfo;
    SIZE_T         Win32ClientInfo[62];
    PVOID          glDispatchTable[233];
    SIZE_T         glReserved1[29];
    PVOID          glReserved2;
    PVOID          glSectionInfo;
    PVOID          glSection;
    PVOID          glTable;
    PVOID          glCurrentRC;
    PVOID          glContext;
    NTSTATUS       LastStatusValue;
    UNICODE_STRING StaticUnicodeString;
    WCHAR          StaticUnicodeBuffer[261];
    PVOID          DeallocationStack;
    PVOID          TlsSlots[64];
    LIST_ENTRY     TlsLinks;
    PVOID          Vdm;
    PVOID          ReservedForNtRpc;
    PVOID          DbgSsReserved[2];
#if (NTDDI_VERSION >= NTDDI_WS03)
    ULONG          HardErrorMode;
#else
    ULONG          HardErrorsAreDisabled;
#endif
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID          Instrumentation[13 - sizeof(GUID)/sizeof(PVOID)];
    GUID           ActivityId;
    PVOID          SubProcessTag;
    PVOID          EtwLocalData;
    PVOID          EtwTraceData;
#elif (NTDDI_VERSION >= NTDDI_WS03)
    PVOID          Instrumentation[14];
    PVOID          SubProcessTag;
    PVOID          EtwLocalData;
#else
    PVOID          Instrumentation[16];
#endif
    PVOID          WinSockData;
    ULONG          GdiBatchCount;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    BOOLEAN        SpareBool0;
    BOOLEAN        SpareBool1;
    BOOLEAN        SpareBool2;
#else
    BOOLEAN        InDbgPrint;
    BOOLEAN        FreeStackOnTermination;
    BOOLEAN        HasFiberData;
#endif
    UCHAR          IdealProcessor;
#if (NTDDI_VERSION >= NTDDI_WS03)
    ULONG          GuaranteedStackBytes;
#else
    ULONG          Spare3;
#endif
    PVOID          ReservedForPerf;
    PVOID          ReservedForOle;
    ULONG          WaitingOnLoaderLock;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID          SavedPriorityState;
    ULONG_PTR      SoftPatchPtr1;
    ULONG_PTR      ThreadPoolData;
#elif (NTDDI_VERSION >= NTDDI_WS03)
    ULONG_PTR      SparePointer1;
    ULONG_PTR      SoftPatchPtr1;
    ULONG_PTR      SoftPatchPtr2;
#else
    Wx86ThreadState Wx86Thread;
#endif
    PVOID*         TlsExpansionSlots;
#if defined(_WIN64) && !defined(EXPLICIT_32BIT)
    PVOID          DeallocationBStore;                                                  
    PVOID          BStoreLimit;                                                         
#endif
    ULONG          ImpersonationLocale;
    ULONG          IsImpersonating;
    PVOID          NlsCache;
    PVOID          pShimData;
    ULONG          HeapVirtualAffinity;
    HANDLE        CurrentTransactionHandle;
    PTEB_ACTIVE_FRAME ActiveFrame;
#if (NTDDI_VERSION >= NTDDI_WS03)
    PVOID FlsData;
#endif
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID PreferredLangauges;
    PVOID UserPrefLanguages;
    PVOID MergedPrefLanguages;
    ULONG MuiImpersonation;
    union
    {
        struct
        {
            USHORT SpareCrossTebFlags:16;
        };
        USHORT CrossTebFlags;
    };
    union
    {
        struct
        {
            USHORT DbgSafeThunkCall:1;
            USHORT DbgInDebugPrint:1;
            USHORT DbgHasFiberData:1;
            USHORT DbgSkipThreadAttach:1;
            USHORT DbgWerInShipAssertCode:1;
            USHORT DbgIssuedInitialBp:1;
            USHORT DbgClonedThread:1;
            USHORT SpareSameTebBits:9;
        };
        USHORT SameTebFlags;
    };
    PVOID TxnScopeEntercallback;
    PVOID TxnScopeExitCAllback;
    PVOID TxnScopeContext;
    ULONG LockCount;
    ULONG ProcessRundown;
    ULONG64 LastSwitchTime;
    ULONG64 TotalSwitchOutTime;
    LARGE_INTEGER WaitReasonBitMap;
#else
    BOOLEAN SafeThunkCall;
    BOOLEAN BooleanSpare[3];
#endif
} TEB_ntexport, *PTEB_ntexport;

#endif // __fisher_ntdll_types_h__
