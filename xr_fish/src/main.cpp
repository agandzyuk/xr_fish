#include "ntdll_exports.h"

#include "app.h"
#include "exception.h"
#include "tools.h"
#include "config.h"

using namespace Fisher;

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    ntdll.LdrReloadExports();

    int code = 0;

    ExceptionBlock_BEGIN();

    Fisher::App* app = new Fisher::App(nCmdShow, &lpCmdLine);
    if ( app )
        app->start();
    ExceptionBlock_END(false, &code);

    return code;
}
