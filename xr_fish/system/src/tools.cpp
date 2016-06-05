#include "ntdll_exports.h"
#include "tools.h"

using namespace Fisher;

/// Mem8 inlines 
template <typename int Sz>
template <typename int Sz2>
Mem8<Sz>::Mem8(const Mem8<Sz2>& copy)
{ 
    *this = copy; 
}

///
template <typename int Sz>
template <typename int Sz2>
inline Mem8<Sz>& 
Mem8<Sz>::operator=(const Mem8<Sz2>& copy)
{
    size_ = min( Sz, Sz2 );
    __asm {
        mov     eax,[this]
        lea     esi,[copy.mem_]
        lea     edi,[eax+size_]
        mov     ecx,[eax+size_]
        rep movsb
        mov     eax,[eax]
    }
}

///
template <typename int Sz>
inline Mem8<Sz>& 
Mem8<Sz>::operator=(const char* copy)
{
    size_ = Sz;
    __asm {
        mov     eax,[this]
        mov     esi,[copy]
        lea     edi,[eax+mem_]
        mov     ecx,[eax+size_]
        rep movsb
        mov     eax,[eax]
    }
}

///
template <typename int Sz>
inline unsigned char
Mem8<Sz>::operator[](unsigned int i)
{
    return mem_[i];
}

///
template <typename int Sz>
inline unsigned char*
Mem8<Sz>::operator&()
{
    return &mem_[i];
}


/// Mem8Interlocked inlines 
template <typename Typ, typename int Sz>
Mem8Interlocked<Typ,Sz>::Mem8Interlocked()
{
    ::InitializeCriticalSection( &cr_ );
}

///
template <typename Typ, typename int Sz>
template <typename int Sz2>
Mem8Interlocked<Typ,Sz>::Mem8Interlocked(const Mem8<Sz2>& copy)
{
    ::InitializeCriticalSection( &cr_ );
    *(Mem8*)this = copy;
}

///
template <typename Typ, typename int Sz>
Mem8Interlocked<Typ,Sz>::~Mem8Interlocked()
{
    ::DeleteCriticalSection( &cr_ );
}

///
template <typename Typ, typename int Sz>
template <typename int Sz2>
inline Mem8Interlocked<Typ,Sz>& 
Mem8Interlocked<Typ,Sz>::operator=(const Mem8<Sz2>& copy)
{
    ::EnterCriticalSection( &cr_ );
    *(Mem8*)this = copy;
    ::LeaveCriticalSection( &cr_ );
    return *this;
}

///
template <typename Typ, typename int Sz>
__forceinline Mem8Interlocked<Typ,Sz>& 
Mem8Interlocked<Typ,Sz>::operator=(const Typ* copy)
{
    ::EnterCriticalSection( &cr_ );
    size_ = Sz;
    __asm {
        mov     eax,[this]
        mov     esi,[copy]
        lea     edi,[eax+mem_]
        mov     bl,type Typ
        cmp     bl,1
        je short __copy_byte
        cmp     bl,2
        je short __copy_word
        cmp     bl,4
        je short __copy_dword
        rep movsb
        jmp short __copy_end

    __copy_byte:
        movsb
        cmp     byte ptr[esi],0
        jnz short __copy_byte
        movsb
        jmp short __copy_end

    __copy_word:
        movsw
        cmp     word ptr[esi],0
        jnz short __copy_word
        movsw
        jmp short __copy_end

    __copy_dword:
        movsd
        cmp     dword ptr[esi],0
        jnz short __copy_dword
        movsd
        jmp short __copy_end

    __copy_end:

        lea     ebx,[eax+mem_]
        sub     edi,ebx
        mov     [eax+size_],edi
        mov     eax,[eax]
    }
    ::LeaveCriticalSection( &cr_ );
}

///
template <typename Typ, typename int Sz>
inline Typ* 
Mem8Interlocked<Typ,Sz>::lock()
{
    ::EnterCriticalSection( &cr_ );
    locked_ = true;
    return (Typ*)&mem_[0];
}

///
template <typename Typ, typename int Sz>
inline void
Mem8Interlocked<Typ,Sz>::unlock()
{
    locked_ = false;
    ::LeaveCriticalSection( &cr_ );
}

///
template <typename Typ, typename int Sz>
inline unsigned int&
Mem8Interlocked<Typ,Sz>::size() 
{
    ::EnterCriticalSection( &cr_ );
    bool locked = locked_;
    ::LeaveCriticalSection( &cr_ );

    if ( !locked )
        throw Exception("Mem8Interlocked::size() unguard request!");
    return size_;
}

/////////////////////////////////////////////////////

namespace Fisher {

/// Text buffer helpers
static Mem8Interlocked< wchar_t, MESSAGE_MAX*2 > InterlockedUnicodeConvBuffer;
static Mem8Interlocked< char, MESSAGE_MAX > InterlockedAnsiConvBuffer;

/// Numeric helpers 
inline const TCHAR* i2str( int val )
{
    bool negative = (val < 0);
    if(negative)
        val *= -1;

    char pos = 63;
    TCHAR *buf = (TCHAR*)InterlockedAnsiConvBuffer.lock();
    buf[pos--] = 0;
    do {
        buf[pos--] = 48 + val%10;
        val = val/10;
    }
    while(0 != val);

    if(negative)
        buf[pos--] = '-';
    InterlockedAnsiConvBuffer.unlock();
    return &buf[pos + 1];
}

///
inline const TCHAR* i2hex( int val, short width )
{
   if ( width <= 0 )
        width = 0;
    width--;
    bool negative = (val < 0);
    if(negative)
        val *= -1;

    short pos = 63;

    TCHAR *buf = (TCHAR*)InterlockedAnsiConvBuffer.lock();
    buf[pos--] = 0;
    do {
        short diff = val%16;
        diff += (diff > 9) ? 55 : 48;
        buf[pos--] = (TCHAR)diff;
        val = val/16;
    }
    while(width-- > 0 || 0 != val);

    if(negative)
        buf[pos--] = _T('-');

    InterlockedAnsiConvBuffer.unlock();
    return &buf[pos + 1];
}

///
inline std::wstring char2wstd(const char* pstr)
{
    ANSI_STRING src;
    src.Length = strlen(pstr);
    src.MaximumLength = src.Length+1;
    src.Buffer = (PCHAR)pstr;

    wchar_t wbuf[ MESSAGE_MAX ];
    UNICODE_STRING dst = {src.Length<<1, src.MaximumLength<<1, wbuf};
    ntdll.RtlAnsiStringToUnicodeString( &dst, &src, 0 );
    return wbuf;
}

///
inline std::string wchar2std(const wchar_t* pstr)
{
    UNICODE_STRING src;
    src.Length = wcslen(pstr)<<1;
    src.MaximumLength = src.Length+2;
    src.Buffer = (PWCHAR)pstr;

    char sbuf[ MESSAGE_MAX ];
    ANSI_STRING dst = {src.Length>>1, src.MaximumLength>>1, sbuf};
    ntdll.RtlUnicodeStringToAnsiString( &dst, &src, 0 );
    return sbuf;
}

///
inline const char* wchar2char(const wchar_t* pstr)
{
    UNICODE_STRING src;
    src.Length = wcslen(pstr)<<1;
    src.MaximumLength = src.Length+2;
    src.Buffer = (PWCHAR)pstr;

    char *pAnsi = InterlockedAnsiConvBuffer.lock();
    ANSI_STRING dst = {src.Length>>1, src.MaximumLength>>1, pAnsi};
    ntdll.RtlUnicodeStringToAnsiString( &dst, &src, 0 );

    InterlockedAnsiConvBuffer.unlock();
    return pAnsi;
}

///
inline const wchar_t* char2wchar(const char* pstr)
{
    ANSI_STRING src;
    src.Length = strlen(pstr);
    src.MaximumLength = src.Length+1;
    src.Buffer = (PCHAR)pstr;

    wchar_t *pUnicode = InterlockedUnicodeConvBuffer.lock();
    UNICODE_STRING dst = {src.Length<<1, src.MaximumLength<<1, pUnicode};
    ntdll.RtlAnsiStringToUnicodeString( &dst, &src, 0 );
    InterlockedAnsiConvBuffer.unlock();
    return pUnicode;
}

///
inline std::wstring std2wstd(const std::string& str)
{
    return char2wstd( str.c_str() );
}

///
inline std::string wstd2std(const std::wstring& str)
{
    return wchar2std( str.c_str() );
}

///
inline const wchar_t* std2wchar(const std::string& str)
{
    return char2wchar( str.c_str() );
}

///
inline const char* wstd2char(const std::wstring& str)
{
    return wchar2char( str.c_str() );
}

///
}
