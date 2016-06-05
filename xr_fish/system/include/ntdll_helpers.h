#ifndef __fisher_ntdll_helpers_h__
#define __fisher_ntdll_helpers_h__

#include "ntdll_types.h"

////////////////////////////////////////////////////////
#define OBJ_INHERIT                             0x00000002L
#define OBJ_PERMANENT                           0x00000010L
#define OBJ_EXCLUSIVE                           0x00000020L
#define OBJ_CASE_INSENSITIVE                    0x00000040L
#define OBJ_OPENIF                              0x00000080L
#define OBJ_OPENLINK                            0x00000100L
#define OBJ_KERNEL_HANDLE                       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK                  0x00000400L
#define OBJ_VALID_ATTRIBUTES                    0x000007F2L

#pragma warning(disable:4005)

#define InitializeObjectAttributes(p,n,a,r,s) { \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES);    \
    (p)->RootDirectory = (r);                   \
    (p)->Attributes = (a);                      \
    (p)->ObjectName = (n);                      \
    (p)->SecurityDescriptor = (s);              \
    (p)->SecurityQualityOfService = NULL;       \
}

#define ZeroObjectAttributes(p) {               \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES);    \
    (p)->RootDirectory = (0);                   \
    (p)->Attributes = (0);                      \
    (p)->ObjectName = (0);                      \
    (p)->SecurityDescriptor = (0);              \
    (p)->SecurityQualityOfService = NULL;       \
}

#pragma warning(default:4005)

////////////////////////////////////////////////////////////////////////////
void WINAPI BaseThreadStartupThunk(void);
void WINAPI BaseProcessStartThunk(void);

////////////////////////////////////////////////////////
namespace Fisher 
{ 
////////////////////////////////////////////////////////
extern void ThreadCreateStack(SIZE_T StackReserve, SIZE_T StackCommit, PINITIAL_TEB InitialTeb);
extern void ThreadFreeStack(PINITIAL_TEB InitialTeb);
extern void ThreadInitializeContext(PCONTEXT Context, PVOID Parameter, PVOID StartAddress, PVOID StackAddress, ULONG ContextType);

}
#endif // __fisher_ntdll_helpers_h__
