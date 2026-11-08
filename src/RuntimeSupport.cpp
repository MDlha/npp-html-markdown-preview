#include "RuntimeSupport.h"
#include <objbase.h>
#include <WebView2.h>
bool CheckPreviewRuntime(HWND owner) {
    LPWSTR version{};
    HRESULT result=GetAvailableCoreWebView2BrowserVersionString(nullptr,&version);
    const bool present=SUCCEEDED(result) && version && version[0] && wcscmp(version,L"0.0.0.0")!=0;
    CoTaskMemFree(version);
    if(!present) MessageBoxW(owner,
        L"Náhľad potrebuje Microsoft Edge WebView2 Runtime. Nie je dostupný.\n\n"
        L"Zavrite Notepad++ a nainštalujte Evergreen WebView2 Runtime z oficiálnej stránky Microsoftu:\n"
        L"https://developer.microsoft.com/en-us/microsoft-edge/webview2/\n"
        L"Odkaz a postup sú aj v README.md pri vydaní 0.8. Potom znova otvorte Notepad++.\n\n"
        L"Visual Studio ani samostatné knižnice Visual C++ nepotrebujete. Doplnok nič nesťahuje ani neinštaluje automaticky.",
        L"Náhľad 0.8 — chýba WebView2",MB_OK|MB_ICONERROR);
    return present;
}
