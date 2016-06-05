#ifndef __fisher_tools_h__
#define __fisher_tools_h__

//#include "lock.h"
#include <windows.h>
#include <tchar.h>
#include <string>

////////////////////////////////////////////////////////
#define ROUND_DOWN(n, align) \
    (((ULONG)n) & ~((align) - 1l))

#define ROUND_UP(n, align) \
    ROUND_DOWN(((ULONG)n) + (align) - 1, (align))

#define MESSAGE_MAX 1024

namespace Fisher {

/// Mem8 may helps in operations with constant un-aligned array
template <typename int Sz>
struct Mem8
{
    Mem8() : size_(0) {};

    template <typename int Sz2>
    Mem8(const Mem8<Sz2>& copy);

    template <typename int Sz2>
    inline Mem8& operator=(const Mem8<Sz2>& copy);
    inline Mem8& operator=(const char* copy);
    inline unsigned char operator[](unsigned int i);
    inline unsigned char* operator&();

    unsigned char mem_[Sz];
    unsigned int  size_;
};

/// Mem8Interlocked is the extension for multithread using
template <typename Typ, int Sz>
struct Mem8Interlocked : private Mem8<Sz>
{
    Mem8Interlocked();

    template <typename int Sz2>
    Mem8Interlocked(const Mem8<Sz2>& copy);

    ~Mem8Interlocked();

    template <typename int Sz2>
    inline Mem8Interlocked& operator=(const Mem8<Sz2>& copy);
    __forceinline Mem8Interlocked& operator=(const Typ* copy);
    inline Typ* lock();
    inline void unlock();
    inline unsigned int& size();

private:
    CRITICAL_SECTION cr_;
    bool locked_;
};

/// Common helpers /////////////////////////////////////////
extern inline const TCHAR*      i2str( int val );
extern inline const TCHAR*      i2hex( int val, short width );

extern inline const char*       wchar2char(const wchar_t* pstr);
extern inline const wchar_t*    char2wchar(const char* pstr);
extern inline std::wstring      char2wstd(const char* pstr);
extern inline std::string       wchar2std(const wchar_t* pstr);
extern inline std::wstring      std2wstd(const std::string& str);
extern inline std::string       wstd2std(const std::wstring& str);
extern inline const wchar_t*    std2wchar(const std::string& str);
extern inline const char*       wstd2char(const std::wstring& str);

}

#endif  // __fisher_tools_h__
