#include "exception.h"
#include "tools.h"

using namespace std;
using namespace Fisher;

const TCHAR Caption[]    = _T("Error");
const TCHAR Unexpected[] = _T("Unexpected internal error!");

/// Exception /////////////////////////////////////////
Exception::Exception( const char* msg )
    : exception( msg ), code_(-1)
{}

Exception::Exception( const string& msg )
    : exception( msg.c_str() ), code_(-1)
{}

Exception::Exception( const wstring& msg )
    : exception("<tag_info_unicode_string>"), unicode_(msg), code_(-1)
{}

Exception::Exception( const exception& stdex, int code )
    : exception( stdex )
{
    code_ = code ? code : -2;
}

Exception::Exception( const Exception& ex )
{
    *this = ex;
}

Exception::Exception( int error_code )
{
#ifdef _WIN32
    *this = Win32Exception( error_code );
#endif
}

int Exception::code() const throw()
{
    return code_;
}

LPCTSTR Exception::reason() const throw()
{
#ifdef UNICODE
    if ( unicode_.empty() ) {
        const_cast<wstring*>(&unicode_)->assign( char2wchar( what() ) );
        return unicode_.c_str();
    }
    return unicode_.c_str();
#else
    return what();
#endif
}

void Exception::handle(bool retry, int* system_error) throw() 
{
    if ( retry ) 
    {
        if ( IDCANCEL == ::MessageBox( NULL, reason(), Caption, MB_OKCANCEL | MB_ICONSTOP ) )
        {
            exit( code_ );
        }
    }
    else 
    {
        ::MessageBox( NULL, reason(), Caption, MB_OK | MB_ICONSTOP );
        exit( code_ );
    }

    if (system_error) 
        *system_error = code_;
}

void Exception::Handle(const Exception& ex, bool retry, int* system_error) throw()
{
    if ( ex.code_ == 0 )
        Exception::Terminate();

    const_cast<Exception*>(&ex)->handle(retry, system_error);
}

void Exception::Terminate() throw()
{
    ::MessageBox( NULL, Unexpected, Caption, MB_OK | MB_ICONSTOP );
    exit( -5 );
}

/// Win32Exception /////////////////////////////////
Win32Exception::Win32Exception(unsigned long error_code) 
    : Exception("<tag_info_unicode_string>")
{
    code_ = -2;

    LPVOID  lpMsgBuf;
    ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            0,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0, NULL );

#ifdef UNICODE
    unicode_ = L"Windows error 0x";
    unicode_ += i2hex(error_code,4);
    unicode_ += L"\n";
    if (0 == lpMsgBuf || 0 == error_code)
        unicode_ += L"<tag_info_not_available>";
    else {
        unicode_ += L"\"";
        unicode_ += (LPCWSTR)lpMsgBuf;
        unicode_ = unicode_.substr(0, unicode_.length()-3 );
        unicode_ += L"\"";
    }
#else
    string msg = "Windows error 0x";
    msg += i2hex(error_code,4);
    msg += "\n";
    if (0 == lpMsgBuf || 0 == error_code)
        msg += "<tag_info_not_available>";
    else {
        msg += "\"";
        msg += (LPCSTR)lpMsgBuf;
        msg = msg.substr(0, msg.length()-3 );
        msg += "\"";
    }
    *(exception*)this = exception( msg.c_str() );
#endif
}

/// UnexpectedException /////////////////////////////////
UnexpectedException::UnexpectedException() 
    : Exception( Unexpected )
{
    code_ = -4;
}
