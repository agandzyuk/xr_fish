#ifndef __fisher_exception_h__
#define __fisher_exception_h__

#include "ntdll_types.h"
#include <string>
#include <assert.h>

#define ExceptionBlock_BEGIN() try {

#define ExceptionBlock_END(retry, system_error)\
    }\
    catch(Exception& ex) {\
        Exception::Handle(ex, retry, system_error);\
    }\
    catch(const std::exception& stdex) {\
        Exception(stdex,-3).handle(retry, system_error);\
    }\
    catch(int error_code) {\
        Exception(error_code).handle(retry, system_error);\
    }\
    catch(...) {\
        UnexpectedException().handle(retry, system_error);\
    }

namespace Fisher {

/// class Exception ///////////////////////////
class Exception : protected std::exception
{
public:
    Exception( const Exception& ex );
    Exception( const std::exception& stdex, int code = 0 );
    Exception( const std::string& msg );
    Exception( const std::wstring& msg );
    Exception( const char* msg );
    Exception( int error_code );
    virtual ~Exception() throw () {}

    virtual int code() const throw();
    virtual LPCTSTR reason() const throw();

    void handle(bool retry, int* system_error) throw();

    // exit application if exception confirmed
    static void Handle(const Exception& ex, bool retry, int* system_error) throw();
    // exit application after error message box
    static void Terminate() throw();

protected:
    int code_;
    std::wstring unicode_;
};

/// class Win32Exception ///////////////////////
class Win32Exception: public Exception
{
public:
    Win32Exception(unsigned long error_code);
};

/// class UnexpectedException ///////////////////////
class UnexpectedException: public Exception
{
public:
    UnexpectedException();
};

}
#endif  // __fisher_exception_h__
