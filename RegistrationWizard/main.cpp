#include "WizardWindow.h"

#pragma comment(lib, "comctl32.lib")

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR cmdLine, int cmdShow) {
    (void)hPrev;
    (void)cmdLine;

    INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    WizardWindow app(hInst);
    if (!app.Init(cmdShow)) {
        return -1;
    }
    return app.Run();
}
