#ifndef _I386_KETYPES_H
#define _I386_KETYPES_H


/////////////////////////////////////////////////////////
// Some from <wdm.h>
/////////////////////////////////////////////////////////
#define GM_LOCK_BIT          0x1
#define GM_LOCK_BIT_V        0x0
#define GM_LOCK_WAITER_WOKEN 0x2
#define GM_LOCK_WAITER_INC   0x4

#define LOCK_QUEUE_WAIT_BIT               0
#define LOCK_QUEUE_OWNER_BIT              1

#define LOCK_QUEUE_WAIT                   1
#define LOCK_QUEUE_OWNER                  2
#define LOCK_QUEUE_TIMER_LOCK_SHIFT       4
#define LOCK_QUEUE_TIMER_TABLE_LOCKS (1 << (8 - LOCK_QUEUE_TIMER_LOCK_SHIFT))


typedef VOID
(NTAPI *PKIPI_WORKER)(
    PVOID PacketContext,
    PVOID Parameter1,
    PVOID Parameter2,
    PVOID Parameter3);

typedef struct _KSPIN_LOCK_QUEUE {
  struct _KSPIN_LOCK_QUEUE *volatile Next;
  PKSPIN_LOCK volatile Lock;
} KSPIN_LOCK_QUEUE, *PKSPIN_LOCK_QUEUE;

typedef struct _KLOCK_QUEUE_HANDLE {
  KSPIN_LOCK_QUEUE LockQueue;
  UCHAR OldIrql;
} KLOCK_QUEUE_HANDLE, *PKLOCK_QUEUE_HANDLE;

#if defined(_AMD64_)

typedef ULONG64 KSPIN_LOCK_QUEUE_NUMBER;

#define LockQueueDispatcherLock 0
#define LockQueueExpansionLock 1
#define LockQueuePfnLock 2
#define LockQueueSystemSpaceLock 3
#define LockQueueVacbLock 4
#define LockQueueMasterLock 5
#define LockQueueNonPagedPoolLock 6
#define LockQueueIoCancelLock 7
#define LockQueueWorkQueueLock 8
#define LockQueueIoVpbLock 9
#define LockQueueIoDatabaseLock 10
#define LockQueueIoCompletionLock 11
#define LockQueueNtfsStructLock 12
#define LockQueueAfdWorkQueueLock 13
#define LockQueueBcbLock 14
#define LockQueueMmNonPagedPoolLock 15
#define LockQueueUnusedSpare16 16
#define LockQueueTimerTableLock 17
#define LockQueueMaximumLock (LockQueueTimerTableLock + LOCK_QUEUE_TIMER_TABLE_LOCKS)

#else

typedef enum _KSPIN_LOCK_QUEUE_NUMBER {
  LockQueueDispatcherLock,
  LockQueueExpansionLock,
  LockQueuePfnLock,
  LockQueueSystemSpaceLock,
  LockQueueVacbLock,
  LockQueueMasterLock,
  LockQueueNonPagedPoolLock,
  LockQueueIoCancelLock,
  LockQueueWorkQueueLock,
  LockQueueIoVpbLock,
  LockQueueIoDatabaseLock,
  LockQueueIoCompletionLock,
  LockQueueNtfsStructLock,
  LockQueueAfdWorkQueueLock,
  LockQueueBcbLock,
  LockQueueMmNonPagedPoolLock,
  LockQueueUnusedSpare16,
  LockQueueTimerTableLock,
  LockQueueMaximumLock = LockQueueTimerTableLock + LOCK_QUEUE_TIMER_TABLE_LOCKS
} KSPIN_LOCK_QUEUE_NUMBER, *PKSPIN_LOCK_QUEUE_NUMBER;

#endif /* defined(_AMD64_) */

//
// Per-Processor Lookaside List
//
typedef struct _PP_LOOKASIDE_LIST
{
    struct _GENERAL_LOOKASIDE *P;
    struct _GENERAL_LOOKASIDE *L;
} PP_LOOKASIDE_LIST, *PPP_LOOKASIDE_LIST;

/////////////////////////////////////////////////////////
// Some from <ketypes.h>
/////////////////////////////////////////////////////////

typedef VOID
(NTAPI KDEFERRED_ROUTINE)(
  struct _KDPC *Dpc,
  PVOID DeferredContext,
  PVOID SystemArgument1,
  PVOID SystemArgument2);
typedef KDEFERRED_ROUTINE *PKDEFERRED_ROUTINE;

typedef struct _KDPC {
    UCHAR Type;
    UCHAR Importance;
    volatile USHORT Number;
    LIST_ENTRY DpcListEntry;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PVOID DeferredContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    volatile PVOID DpcData;
} KDPC, *PKDPC, *RESTRICTED_POINTER PRKDPC;


#define TIMER_EXPIRED_INDEX_BITS        6
#define TIMER_PROCESSOR_INDEX_BITS      5

typedef struct _DISPATCHER_HEADER {
  union {
    struct {
      UCHAR Type;
      union {
        union {
          UCHAR TimerControlFlags;
          struct DUMMYSTRUCTNAME {
            UCHAR Absolute:1;
            UCHAR Coalescable:1;
            UCHAR KeepShifting:1;
            UCHAR EncodedTolerableDelay:5;
          };
        } DUMMYUNIONNAME;
        UCHAR Abandoned;
#if (NTDDI_VERSION < NTDDI_WIN7)
        UCHAR NpxIrql;
#endif
        BOOLEAN Signalling;
      } DUMMYUNIONNAME;
      union {
        union {
          UCHAR ThreadControlFlags;
          struct DUMMYSTRUCTNAME {
            UCHAR CpuThrottled:1;
            UCHAR CycleProfiling:1;
            UCHAR CounterProfiling:1;
            UCHAR Reserved:5;
          } ;
        } DUMMYUNIONNAME;
        UCHAR Size;
        UCHAR Hand;
      } DUMMYUNIONNAME2;
      union {
#if (NTDDI_VERSION >= NTDDI_WIN7)
        union {
          UCHAR TimerMiscFlags;
          struct DUMMYSTRUCTNAME {
#if !defined(_X86_)
            UCHAR Index:TIMER_EXPIRED_INDEX_BITS;
#else
            UCHAR Index:1;
            UCHAR Processor:TIMER_PROCESSOR_INDEX_BITS;
#endif
            UCHAR Inserted:1;
            volatile UCHAR Expired:1;
          };
        } DUMMYUNIONNAME;
#else
        /* Pre Win7 compatibility fix to latest WDK */
        UCHAR Inserted;
#endif
        union {
          BOOLEAN DebugActive;
          struct DUMMYSTRUCTNAME {
            BOOLEAN ActiveDR7:1;
            BOOLEAN Instrumented:1;
            BOOLEAN Reserved2:4;
            BOOLEAN UmsScheduled:1;
            BOOLEAN UmsPrimary:1;
          };
        } DUMMYUNIONNAME; /* should probably be DUMMYUNIONNAME2, but this is what WDK says */
        BOOLEAN DpcActive;
      } DUMMYUNIONNAME3;
    } DUMMYSTRUCTNAME;
    volatile LONG Lock;
  } DUMMYUNIONNAME;
  LONG SignalState;
  LIST_ENTRY WaitListHead;
} DISPATCHER_HEADER, *PDISPATCHER_HEADER;

//
// Kernel Event
//
typedef struct _KEVENT {
  DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT, *RESTRICTED_POINTER PRKEVENT;

//
// Kernel Event Pair Object
//
typedef struct _KEVENT_PAIR
{
    short Type;
    short Size;
    KEVENT LowEvent;
    KEVENT HighEvent;
} KEVENT_PAIR, *PKEVENT_PAIR;

//
// PRCB DPC Data
//
typedef struct _KDPC_DATA
{
    LIST_ENTRY DpcListHead;
    ULONG_PTR DpcLock;
#ifdef _M_AMD64
    volatile LONG DpcQueueDepth;
#else
    volatile ULONG DpcQueueDepth;
#endif
    ULONG DpcCount;
} KDPC_DATA, *PKDPC_DATA;

//
// KPCR Access for non-IA64 builds
//
#define K0IPCR                  ((ULONG_PTR)(KIP0PCRADDRESS))
#define PCR                     ((KPCR *)K0IPCR)
#if defined(CONFIG_SMP) || defined(NT_BUILD)
#undef  KeGetPcr
#define KeGetPcr()              ((KPCR *)__readfsdword(FIELD_OFFSET(KPCR, SelfPcr)))
#endif

//
// Machine Types
//
#define MACHINE_TYPE_ISA        0x0000
#define MACHINE_TYPE_EISA       0x0001
#define MACHINE_TYPE_MCA        0x0002

//
// X86 80386 Segment Types
//
#define I386_TASK_GATE          0x5
#define I386_TSS                0x9
#define I386_ACTIVE_TSS         0xB
#define I386_CALL_GATE          0xC
#define I386_INTERRUPT_GATE     0xE
#define I386_TRAP_GATE          0xF

//
// Selector Names
//
#define RPL_MASK                0x0003
#define MODE_MASK               0x0001
#define KGDT_R0_CODE            0x8
#define KGDT_R0_DATA            0x10
#define KGDT_R3_CODE            0x18
#define KGDT_R3_DATA            0x20
#define KGDT_TSS                0x28
#define KGDT_R0_PCR             0x30
#define KGDT_R3_TEB             0x38
#define KGDT_LDT                0x48
#define KGDT_DF_TSS             0x50
#define KGDT_NMI_TSS            0x58

//
// Define the number of GDTs that can be queried by user mode
//
#define KGDT_NUMBER             10

//
// CR4
//
#define CR4_VME                 0x1
#define CR4_PVI                 0x2
#define CR4_TSD                 0x4
#define CR4_DE                  0x8
#define CR4_PSE                 0x10
#define CR4_PAE                 0x20
#define CR4_MCE                 0x40
#define CR4_PGE                 0x80
#define CR4_FXSR                0x200
#define CR4_XMMEXCPT            0x400

//
// EFlags
//
#define EFLAGS_CF               0x01L
#define EFLAGS_ZF               0x40L
#define EFLAGS_TF               0x100L
#define EFLAGS_INTERRUPT_MASK   0x200L
#define EFLAGS_DF               0x400L
#define EFLAGS_IOPL             0x3000L
#define EFLAGS_NESTED_TASK      0x4000L
#define EFLAGS_RF               0x10000
#define EFLAGS_V86_MASK         0x20000
#define EFLAGS_ALIGN_CHECK      0x40000
#define EFLAGS_VIF              0x80000
#define EFLAGS_VIP              0x100000
#define EFLAGS_ID               0x200000
#define EFLAGS_USER_SANITIZE    0x3F4DD7
#define EFLAG_SIGN              0x8000
#define EFLAG_ZERO              0x4000

//
// Legacy floating status word bit masks.
//
#define FSW_INVALID_OPERATION   0x1
#define FSW_DENORMAL            0x2
#define FSW_ZERO_DIVIDE         0x4
#define FSW_OVERFLOW            0x8
#define FSW_UNDERFLOW           0x10
#define FSW_PRECISION           0x20
#define FSW_STACK_FAULT         0x40

//
// IPI Types
//
#define IPI_APC                 1
#define IPI_DPC                 2
#define IPI_FREEZE              4
#define IPI_PACKET_READY        8
#define IPI_SYNCH_REQUEST       16

//
// PRCB Flags
//
#define PRCB_MAJOR_VERSION      1
#define PRCB_BUILD_DEBUG        1
#define PRCB_BUILD_UNIPROCESSOR 2

//
// HAL Variables
//
#define INITIAL_STALL_COUNT     100
#ifdef PAE
#define HYPERSPACE_BASE         0xc0400000
#else
#define HYPERSPACE_BASE         0xc0800000
#endif
#define MM_HAL_VA_START         0xFFC00000
#define MM_HAL_VA_END           0xFFFFFFFF
#define APIC_BASE               0xFFFE0000

//
// IOPM Definitions
//
#define IOPM_COUNT              1
#define IOPM_SIZE               8192
#define IOPM_FULL_SIZE          8196
#define IO_ACCESS_MAP_NONE      0
#define IOPM_DIRECTION_MAP_SIZE 32
#define IOPM_OFFSET             FIELD_OFFSET(KTSS, IoMaps[0].IoMap)
#define KiComputeIopmOffset(MapNumber)              \
    (MapNumber == IO_ACCESS_MAP_NONE) ?             \
        (USHORT)(sizeof(KTSS)) :                    \
        (USHORT)(FIELD_OFFSET(KTSS, IoMaps[MapNumber-1].IoMap))

typedef UCHAR KIO_ACCESS_MAP[IOPM_SIZE];

typedef KIO_ACCESS_MAP *PKIO_ACCESS_MAP;

//
// Size of the XMM register save area in the FXSAVE format
//
#define SIZE_OF_FX_REGISTERS    128

//
// Static Kernel-Mode Address start (use MM_KSEG0_BASE for actual)
//
#define KSEG0_BASE              0x80000000

//
// Synchronization-level IRQL
//
#ifndef CONFIG_SMP
#define SYNCH_LEVEL             DISPATCH_LEVEL
#else
#if (NTDDI_VERSION < NTDDI_WS03)
#define SYNCH_LEVEL             (IPI_LEVEL - 1)
#else
#define SYNCH_LEVEL             (IPI_LEVEL - 2)
#endif
#endif

//
// Number of pool lookaside lists per pool in the PRCB
//
#define NUMBER_POOL_LOOKASIDE_LISTS 32

//
// Trap Frame Definition
//
typedef struct _KTRAP_FRAME
{
    ULONG DbgEbp;
    ULONG DbgEip;
    ULONG DbgArgMark;
    ULONG DbgArgPointer;
    ULONG TempSegCs;
    ULONG TempEsp;
    ULONG Dr0;
    ULONG Dr1;
    ULONG Dr2;
    ULONG Dr3;
    ULONG Dr6;
    ULONG Dr7;
    ULONG SegGs;
    ULONG SegEs;
    ULONG SegDs;
    ULONG Edx;
    ULONG Ecx;
    ULONG Eax;
    ULONG PreviousPreviousMode;
    struct _EXCEPTION_REGISTRATION_RECORD FAR *ExceptionList;
    ULONG SegFs;
    ULONG Edi;
    ULONG Esi;
    ULONG Ebx;
    ULONG Ebp;
    ULONG ErrCode;
    ULONG Eip;
    ULONG SegCs;
    ULONG EFlags;
    ULONG HardwareEsp;
    ULONG HardwareSegSs;
    ULONG V86Es;
    ULONG V86Ds;
    ULONG V86Fs;
    ULONG V86Gs;
} KTRAP_FRAME, *PKTRAP_FRAME;

//
// Defines the Callback Stack Layout for User Mode Callbacks
//
typedef struct _KCALLOUT_FRAME
{
    ULONG InitialStack;
    ULONG TrapFrame;
    ULONG CallbackStack;
    ULONG Edi;
    ULONG Esi;
    ULONG Ebx;
    ULONG Ebp;
    ULONG ReturnAddress;
    ULONG Result;
    ULONG ResultLength;
} KCALLOUT_FRAME, *PKCALLOUT_FRAME;

//
// KTIMER Struct Definition
//

typedef struct _KTIMER {
  DISPATCHER_HEADER Header;
  ULARGE_INTEGER DueTime;
  LIST_ENTRY TimerListEntry;
  struct _KDPC *Dpc;
#if (NTDDI_VERSION >= NTDDI_WIN7) && !defined(_X86_)
  ULONG Processor;
#endif
  ULONG Period;
} KTIMER, *PKTIMER, *RESTRICTED_POINTER PRKTIMER;

//
// LDT Entry Definition
//
#ifndef _LDT_ENTRY_DEFINED
#define _LDT_ENTRY_DEFINED
typedef struct _LDT_ENTRY
{
    USHORT LimitLow;
    USHORT BaseLow;
    union
    {
        struct
        {
            UCHAR BaseMid;
            UCHAR Flags1;
            UCHAR Flags2;
            UCHAR BaseHi;
        } Bytes;
        struct
        {
            ULONG BaseMid:8;
            ULONG Type:5;
            ULONG Dpl:2;
            ULONG Pres:1;
            ULONG LimitHi:4;
            ULONG Sys:1;
            ULONG Reserved_0:1;
            ULONG Default_Big:1;
            ULONG Granularity:1;
            ULONG BaseHi:8;
        } Bits;
    } HighWord;
} LDT_ENTRY, *PLDT_ENTRY, *LPLDT_ENTRY;
#endif

//
// GDT Entry Definition
//
typedef struct _KGDTENTRY
{
    USHORT LimitLow;
    USHORT BaseLow;
    union
    {
        struct
        {
            UCHAR BaseMid;
            UCHAR Flags1;
            UCHAR Flags2;
            UCHAR BaseHi;
        } Bytes;
        struct
        {
            ULONG BaseMid:8;
            ULONG Type:5;
            ULONG Dpl:2;
            ULONG Pres:1;
            ULONG LimitHi:4;
            ULONG Sys:1;
            ULONG Reserved_0:1;
            ULONG Default_Big:1;
            ULONG Granularity:1;
            ULONG BaseHi:8;
        } Bits;
    } HighWord;
} KGDTENTRY, *PKGDTENTRY;

//
// IDT Entry Access Definition
//
typedef struct _KIDT_ACCESS
{
    union
    {
        struct
        {
            UCHAR Reserved;
            UCHAR SegmentType:4;
            UCHAR SystemSegmentFlag:1;
            UCHAR Dpl:2;
            UCHAR Present:1;
        };
        USHORT Value;
    };
} KIDT_ACCESS, *PKIDT_ACCESS;

//
// IDT Entry Definition
//
typedef struct _KIDTENTRY
{
    USHORT Offset;
    USHORT Selector;
    USHORT Access;
    USHORT ExtendedOffset;
} KIDTENTRY, *PKIDTENTRY;

typedef struct _DESCRIPTOR
{
    USHORT Pad;
    USHORT Limit;
    ULONG Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

#ifndef NTOS_MODE_USER

//
// FN/FX (FPU) Save Area Structures
//
typedef struct _FNSAVE_FORMAT
{
    ULONG ControlWord;
    ULONG StatusWord;
    ULONG TagWord;
    ULONG ErrorOffset;
    ULONG ErrorSelector;
    ULONG DataOffset;
    ULONG DataSelector;
    UCHAR RegisterArea[80];
} FNSAVE_FORMAT, *PFNSAVE_FORMAT;

typedef struct _FXSAVE_FORMAT
{
    USHORT ControlWord;
    USHORT StatusWord;
    USHORT TagWord;
    USHORT ErrorOpcode;
    ULONG ErrorOffset;
    ULONG ErrorSelector;
    ULONG DataOffset;
    ULONG DataSelector;
    ULONG MXCsr;
    ULONG MXCsrMask;
    UCHAR RegisterArea[SIZE_OF_FX_REGISTERS];
    UCHAR Reserved3[128];
    UCHAR Reserved4[224];
    UCHAR Align16Byte[8];
} FXSAVE_FORMAT, *PFXSAVE_FORMAT;

typedef struct _FX_SAVE_AREA
{
    union
    {
        FNSAVE_FORMAT FnArea;
        FXSAVE_FORMAT FxArea;
    } U;
    ULONG NpxSavedCpu;
    ULONG Cr0NpxState;
} FX_SAVE_AREA, *PFX_SAVE_AREA;

//
// Special Registers Structure (outside of CONTEXT)
//
typedef struct _KSPECIAL_REGISTERS
{
    ULONG Cr0;
    ULONG Cr2;
    ULONG Cr3;
    ULONG Cr4;
    ULONG KernelDr0;
    ULONG KernelDr1;
    ULONG KernelDr2;
    ULONG KernelDr3;
    ULONG KernelDr6;
    ULONG KernelDr7;
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG Reserved[6];
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Processor State Data
//
typedef struct _KPROCESSOR_STATE
{
    CONTEXT ContextFrame;
    KSPECIAL_REGISTERS SpecialRegisters;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;


#define POWER_PERF_SCALE                  100
#define PERF_LEVEL_TO_PERCENT(x)          (((x) * 1000) / (POWER_PERF_SCALE * 10))
#define PERCENT_TO_PERF_LEVEL(x)          (((x) * POWER_PERF_SCALE * 10) / 1000)

typedef struct _PROCESSOR_IDLE_TIMES {
  ULONGLONG StartTime;
  ULONGLONG EndTime;
  ULONG IdleHandlerReserved[4];
} PROCESSOR_IDLE_TIMES, *PPROCESSOR_IDLE_TIMES;

//
// Processor Power State Data
//
struct _PROCESSOR_POWER_STATE;

typedef struct _PROCESSOR_PERF_STATE {
    UCHAR PercentFrequency;
    UCHAR MinCapacity;
    USHORT Power;
    UCHAR IncreaseLevel;
    UCHAR DecreaseLevel;
    USHORT Flags;
    ULONG IncreaseTime;
    ULONG DecreaseTime;
    ULONG IncreaseCount;
    ULONG DecreaseCount;
    ULONGLONG PerformanceTime;
} PROCESSOR_PERF_STATE, *PPROCESSOR_PERF_STATE;

typedef
VOID
(__fastcall * PPROCESSOR_IDLE_FUNCTION)(
    struct _PROCESSOR_POWER_STATE *PState);

typedef struct _PROCESSOR_POWER_STATE
{
    PPROCESSOR_IDLE_FUNCTION IdleFunction;
    ULONG Idle0KernelTimeLimit;
    ULONG Idle0LastTime;
    PVOID IdleHandlers;
    PVOID IdleState;
    ULONG IdleHandlersCount;
    ULONGLONG LastCheck;
    PROCESSOR_IDLE_TIMES IdleTimes;
    ULONG IdleTime1;
    ULONG PromotionCheck;
    ULONG IdleTime2;
    UCHAR CurrentThrottle;
    UCHAR ThermalThrottleLimit;
    UCHAR CurrentThrottleIndex;
    UCHAR ThermalThrottleIndex;
    ULONG LastKernelUserTime;
    ULONG PerfIdleTime;
    ULONGLONG DebugDelta;
    ULONG DebugCount;
    ULONG LastSysTime;
    ULONGLONG TotalIdleStateTime[3];
    ULONG TotalIdleTransitions[3];
    ULONGLONG PreviousC3StateTime;
    UCHAR KneeThrottleIndex;
    UCHAR ThrottleLimitIndex;
    UCHAR PerfStatesCount;
    UCHAR ProcessorMinThrottle;
    UCHAR ProcessorMaxThrottle;
    UCHAR LastBusyPercentage;
    UCHAR LastC3Percentage;
    UCHAR LastAdjustedBusyPercentage;
    ULONG PromotionCount;
    ULONG DemotionCount;
    ULONG ErrorCount;
    ULONG RetryCount;
    ULONG Flags;
    LARGE_INTEGER PerfCounterFrequency;
    ULONG PerfTickCount;
    KTIMER PerfTimer;
    KDPC PerfDpc;
    PROCESSOR_PERF_STATE *PerfStates;
    PVOID PerfSetThrottle;
    ULONG LastC3KernelUserTime;
    ULONG Spare1[1];
} PROCESSOR_POWER_STATE, *PPROCESSOR_POWER_STATE;

//
// Processor Region Control Block
//
#pragma pack(push,4)
typedef struct _KPRCB
{
    USHORT MinorVersion;
    USHORT MajorVersion;
    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;
    UCHAR Number;
    UCHAR Reserved;
    USHORT BuildType;
    KAFFINITY SetMember;
    UCHAR CpuType;
    UCHAR CpuID;
    USHORT CpuStep;
    KPROCESSOR_STATE ProcessorState;
    ULONG KernelReserved[16];
    ULONG HalReserved[16];
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG CFlushSize;
    UCHAR PrcbPad0[88];
#else
    UCHAR PrcbPad0[92];
#endif
    KSPIN_LOCK_QUEUE LockQueue[LockQueueMaximumLock];
    struct _KTHREAD *NpxThread;
    ULONG InterruptCount;
    ULONG KernelTime;
    ULONG UserTime;
    ULONG DpcTime;
    ULONG DebugDpcTime;
    ULONG InterruptTime;
    ULONG AdjustDpcThreshold;
    ULONG PageColor;
    UCHAR SkipTick;
    UCHAR DebuggerSavedIRQL;
#if (NTDDI_VERSION >= NTDDI_WS03)
    UCHAR NodeColor;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UCHAR PollSlot;
#else
    UCHAR Spare1;
#endif
    ULONG NodeShiftedColor;
#else
    UCHAR Spare1[6];
#endif
    struct _KNODE *ParentNode;
    ULONG MultiThreadProcessorSet;
    struct _KPRCB *MultiThreadSetMaster;
#if (NTDDI_VERSION >= NTDDI_WS03)
    ULONG SecondaryColorMask;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG DpcTimeLimit;
#else
    LONG Sleeping;
#endif
#else
    ULONG ThreadStartCount[2];
#endif
    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadNotPossible;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;
#if (NTDDI_VERSION < NTDDI_LONGHORN)
    ULONG KeAlignmentFixupCount;
#endif
    ULONG SpareCounter0;
#if (NTDDI_VERSION < NTDDI_LONGHORN)
    ULONG KeDcacheFlushCount;
    ULONG KeExceptionDispatchCount;
    ULONG KeFirstLevelTbFills;
    ULONG KeFloatingEmulationCount;
    ULONG KeIcacheFlushCount;
    ULONG KeSecondLevelTbFills;
    ULONG KeSystemCalls;
#endif
    volatile ULONG IoReadOperationCount;
    volatile ULONG IoWriteOperationCount;
    volatile ULONG IoOtherOperationCount;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG CcFastMdlReadNoWait;
    ULONG CcFastMdlReadWait;
    ULONG CcFastMdlReadNotPossible;
    ULONG CcMapDataNoWait;
    ULONG CcMapDataWait;
    ULONG CcPinMappedDataCount;
    ULONG CcPinReadNoWait;
    ULONG CcPinReadWait;
    ULONG CcMdlReadNoWait;
    ULONG CcMdlReadWait;
    ULONG CcLazyWriteHotSpots;
    ULONG CcLazyWriteIos;
    ULONG CcLazyWritePages;
    ULONG CcDataFlushes;
    ULONG CcDataPages;
    ULONG CcLostDelayedWrites;
    ULONG CcFastReadResourceMiss;
    ULONG CcCopyReadWaitMiss;
    ULONG CcFastMdlReadResourceMiss;
    ULONG CcMapDataNoWaitMiss;
    ULONG CcMapDataWaitMiss;
    ULONG CcPinReadNoWaitMiss;
    ULONG CcPinReadWaitMiss;
    ULONG CcMdlReadNoWaitMiss;
    ULONG CcMdlReadWaitMiss;
    ULONG CcReadAheadIos;
    ULONG KeAlignmentFixupCount;
    ULONG KeExceptionDispatchCount;
    ULONG KeSystemCalls;
    ULONG PrcbPad1[3];
#else
    ULONG SpareCounter1[8];
#endif
    PP_LOOKASIDE_LIST PPLookasideList[16];
    PP_LOOKASIDE_LIST PPNPagedLookasideList[NUMBER_POOL_LOOKASIDE_LISTS];
    PP_LOOKASIDE_LIST PPPagedLookasideList[NUMBER_POOL_LOOKASIDE_LISTS];
    volatile ULONG PacketBarrier;
    volatile ULONG ReverseStall;
    PVOID IpiFrame;
    UCHAR PrcbPad2[52];
    volatile PVOID CurrentPacket[3];
    volatile ULONG TargetSet;
    volatile PKIPI_WORKER WorkerRoutine;
    volatile ULONG IpiFrozen;
    UCHAR PrcbPad3[40];
    volatile ULONG RequestSummary;
    volatile struct _KPRCB *SignalDone;
    UCHAR PrcbPad4[56];
    struct _KDPC_DATA DpcData[2];
    PVOID DpcStack;
    ULONG MaximumDpcQueueDepth;
    ULONG DpcRequestRate;
    ULONG MinimumDpcRate;
    volatile UCHAR DpcInterruptRequested;
    volatile UCHAR DpcThreadRequested;
    volatile UCHAR DpcRoutineActive;
    volatile UCHAR DpcThreadActive;
    ULONG PrcbLock;
    ULONG DpcLastCount;
    volatile ULONG TimerHand;
    volatile ULONG TimerRequest;
    PVOID DpcThread;
    KEVENT DpcEvent;
    UCHAR ThreadDpcEnable;
    volatile BOOLEAN QuantumEnd;
    UCHAR PrcbPad50;
    volatile UCHAR IdleSchedule;
    LONG DpcSetEventRequest;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    LONG Sleeping;
    ULONG PeriodicCount;
    ULONG PeriodicBias;
    UCHAR PrcbPad5[6];
#else
    UCHAR PrcbPad5[18];
#endif
    LONG TickOffset;
    KDPC CallDpc;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    LONG ClockKeepAlive;
    UCHAR ClockCheckSlot;
    UCHAR ClockPollCycle;
    UCHAR PrcbPad6[2];
    LONG DpcWatchdogPeriod;
    LONG DpcWatchDogCount;
    LONG ThreadWatchdogPeriod;
    LONG ThreadWatchDogCount;
    ULONG PrcbPad70[2];
#else
    ULONG PrcbPad7[8];
#endif
    LIST_ENTRY WaitListHead;
    ULONG ReadySummary;
    ULONG QueueIndex;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    SINGLE_LIST_ENTRY DeferredReadyListHead;
    ULONGLONG StartCycles;
    ULONGLONG CycleTime;
    ULONGLONG PrcbPad71[3];
    LIST_ENTRY DispatcherReadyListHead[32];
#else
    LIST_ENTRY DispatcherReadyListHead[32];
    SINGLE_LIST_ENTRY DeferredReadyListHead;
    ULONG PrcbPad72[11];
#endif
    PVOID ChainedInterruptList;
    LONG LookasideIrpFloat;
    volatile LONG MmPageFaultCount;
    volatile LONG MmCopyOnWriteCount;
    volatile LONG MmTransitionCount;
    volatile LONG MmCacheTransitionCount;
    volatile LONG MmDemandZeroCount;
    volatile LONG MmPageReadCount;
    volatile LONG MmPageReadIoCount;
    volatile LONG MmCacheReadCount;
    volatile LONG MmCacheIoCount;
    volatile LONG MmDirtyPagesWriteCount;
    volatile LONG MmDirtyWriteIoCount;
    volatile LONG MmMappedPagesWriteCount;
    volatile LONG MmMappedWriteIoCount;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG CachedCommit;
    ULONG CachedResidentAvailable;
    PVOID HyperPte;
    UCHAR CpuVendor;
    UCHAR PrcbPad9[3];
#else
    ULONG SpareFields0[1];
#endif
    CHAR VendorString[13];
    UCHAR InitialApicId;
    UCHAR LogicalProcessorsPerPhysicalProcessor;
    ULONG MHz;
    ULONG FeatureBits;
    LARGE_INTEGER UpdateSignature;
    volatile LARGE_INTEGER IsrTime;
    LARGE_INTEGER SpareField1;
    FX_SAVE_AREA NpxSaveArea;
    PROCESSOR_POWER_STATE PowerState;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    KDPC DpcWatchdogDoc;
    KTIMER DpcWatchdogTimer;
    PVOID WheaInfo;
    PVOID EtwSupport;
    SLIST_HEADER InterruptObjectPool;
    LARGE_INTEGER HyperCallPagePhysical;
    LARGE_INTEGER HyperCallPageVirtual;
    PVOID RateControl;
    CACHE_DESCRIPTOR Cache[5];
    ULONG CacheCount;
    ULONG CacheProcessorMask[5];
    UCHAR LogicalProcessorsPerCore;
    UCHAR PrcbPad8[3];
    ULONG PackageProcessorSet;
    ULONG CoreProcessorSet;
#endif
} KPRCB, *PKPRCB;

typedef struct _KFLOATING_SAVE {
  ULONG Dummy;
} KFLOATING_SAVE, *PKFLOATING_SAVE;

typedef struct _KPCR_TIB {
    PVOID ExceptionList;            /* 00 */
    PVOID StackBase;                /* 04 */
    PVOID StackLimit;               /* 08 */
    PVOID SubSystemTib;             /* 0C */
    union {
        PVOID FiberData;            /* 10 */
        ULONG Version;              /* 10 */
    } DUMMYUNIONNAME;
    PVOID ArbitraryUserPointer;     /* 14 */
    struct _KPCR_TIB *Self;         /* 18 */
} KPCR_TIB, *PKPCR_TIB;             /* 1C */

typedef struct _KPCR {
    KPCR_TIB Tib;                /* 00 */
    struct _KPCR *Self;          /* 1C */
    struct _KPRCB *Prcb;         /* 20 */
    UCHAR Irql;                  /* 24 */
    ULONG IRR;                   /* 28 */
    ULONG IrrActive;             /* 2C */
    ULONG IDR;                   /* 30 */
    PVOID KdVersionBlock;        /* 34 */
    PUSHORT IDT;                 /* 38 */
    PUSHORT GDT;                 /* 3C */
    struct _KTSS *TSS;           /* 40 */
    USHORT MajorVersion;         /* 44 */
    USHORT MinorVersion;         /* 46 */
    KAFFINITY SetMember;         /* 48 */
    ULONG StallScaleFactor;      /* 4C */
    UCHAR SpareUnused;           /* 50 */
    UCHAR Number;                /* 51 */
} KPCR, *PKPCR;                  /* 54 */

//
// Macro to get current KPRCB
//
FORCEINLINE
struct _KPRCB *
KeGetCurrentPrcb(VOID)
{
    return (struct _KPRCB *)(ULONG_PTR)__readfsdword(FIELD_OFFSET(KPCR, Prcb));
}

//
// Processor Control Region
//
typedef struct _KIPCR
{
    union
    {
        NT_TIB NtTib;
        struct
        {
            struct _EXCEPTION_REGISTRATION_RECORD *Used_ExceptionList;
            PVOID Used_StackBase;
            PVOID PerfGlobalGroupMask;
            PVOID TssCopy;
            ULONG ContextSwitches;
            KAFFINITY SetMemberCopy;
            PVOID Used_Self;
        };
    };
    struct _KPCR *Self;
    struct _KPRCB *Prcb;
    UCHAR Irql;
    ULONG IRR;
    ULONG IrrActive;
    ULONG IDR;
    PVOID KdVersionBlock;
    PKIDTENTRY IDT;
    PKGDTENTRY GDT;
    struct _KTSS *TSS;
    USHORT MajorVersion;
    USHORT MinorVersion;
    KAFFINITY SetMember;
    ULONG StallScaleFactor;
    UCHAR SpareUnused;
    UCHAR Number;
    UCHAR Spare0;
    UCHAR SecondLevelCacheAssociativity;
    ULONG VdmAlert;
    ULONG KernelReserved[14];
    ULONG SecondLevelCacheSize;
    ULONG HalReserved[16];
    ULONG InterruptMode;
    UCHAR Spare1;
    ULONG KernelReserved2[17];
    KPRCB PrcbData;
} KIPCR, *PKIPCR;
#pragma pack(pop)

//
// TSS Definition
//
typedef struct _KiIoAccessMap
{
    UCHAR DirectionMap[IOPM_DIRECTION_MAP_SIZE];
    UCHAR IoMap[IOPM_FULL_SIZE];
} KIIO_ACCESS_MAP;

typedef struct _KTSS
{
    USHORT Backlink;
    USHORT Reserved0;
    ULONG Esp0;
    USHORT Ss0;
    USHORT Reserved1;
    ULONG NotUsed1[4];
    ULONG CR3;
    ULONG Eip;
    ULONG EFlags;
    ULONG Eax;
    ULONG Ecx;
    ULONG Edx;
    ULONG Ebx;
    ULONG Esp;
    ULONG Ebp;
    ULONG Esi;
    ULONG Edi;
    USHORT Es;
    USHORT Reserved2;
    USHORT Cs;
    USHORT Reserved3;
    USHORT Ss;
    USHORT Reserved4;
    USHORT Ds;
    USHORT Reserved5;
    USHORT Fs;
    USHORT Reserved6;
    USHORT Gs;
    USHORT Reserved7;
    USHORT LDT;
    USHORT Reserved8;
    USHORT Flags;
    USHORT IoMapBase;
    KIIO_ACCESS_MAP IoMaps[IOPM_COUNT];
    UCHAR IntDirectionMap[IOPM_DIRECTION_MAP_SIZE];
} KTSS, *PKTSS;

//
// i386 CPUs don't have exception frames
//
typedef struct _KEXCEPTION_FRAME KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;
#endif
#endif
