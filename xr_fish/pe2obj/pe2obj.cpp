#include "pe2obj.h"
#include "tools.h"
#include "exception.h"
#include <conio.h>

using namespace std;
using namespace Fisher;


struct Assembly
{
    Assembly() : handle_(0)
    {}
/*
    HMODULE load(const char* pe_name);
    {
        module = ::LoadLibraryA(pe_name);
        return module;
    }
*/
    HMODULE handle_;
};

int main(int argc, char* argv[])
{
    try {
        ntdll.reload();
    }
    catch( const Exception& ex ) {
        ::MessageBox(0,  ex.reason(), _T("Error"), 16);
        exit( -1 );
    }
    catch( const std::exception& ex ) {
        ::MessageBox(0,  Exception(ex).reason(), _T("STD error"), 16);
        exit( -2 );
    }
    catch( ... ) {
        ::MessageBox(0,  UnexpectedException().reason(), _T("Unexpected error"), 16);
        exit( -3 );
    }
    printf("Press a key to exit...");
    _getch();
	return 0;
}

