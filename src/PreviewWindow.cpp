#include "PreviewWindow.h"
#include "RuntimeSupport.h"
#include <objbase.h>
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <wrl.h>
#include <shlobj.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Callback;
namespace {
constexpr wchar_t ClassName[] = L"AccessiblePreview.Probe";
constexpr UINT CloseCommand = 100;
constexpr UINT FocusContent = 101;
struct State {
    HWND window{};
    HWND owner{};
    HWND previousFocus{};
    bool quit{};
    bool initialDocumentPending{true};
    bool navigationBlocked{};
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> web;
};
std::vector<std::shared_ptr<State>> windows;
// No remote resources, user input, or editor bridge in the accessibility probe.
constexpr wchar_t Article[] = LR"HTML(<!doctype html>
<html lang="sk"><head><meta charset="utf-8"><title>Skúška prístupnosti náhľadu</title>
<meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src 'unsafe-inline'; script-src 'unsafe-inline'; form-action 'none'; frame-src 'none'">
<style>body{font:1.15rem system-ui;line-height:1.6;max-width:65rem;margin:2rem auto;padding:0 1rem}button,a{font:inherit}table{border-collapse:collapse}td,th{border:1px solid;padding:.5rem}:focus-visible{outline:3px solid Highlight} @media(prefers-color-scheme:dark){body{background:#171717;color:#eee}a{color:#9dcaff}}</style></head>
<body tabindex="-1"><header><h1>Skúška prístupnosti náhľadu</h1><p>Toto je skúšobný článok, nie obsah editora.</p></header>
<nav aria-label="Obsah"><a href="#tabulka">Prejsť na tabuľku</a></nav>
<main><h2>Navigácia s NVDA</h2><p>Vyskúšajte nadpisy, odkazy, tabuľky, oblasti a zoznam prvkov.</p>
<ul><li>Enter aktivuje odkaz.</li><li>Medzerník aktivuje tlačidlo.</li><li>Alt+F4 zatvorí okno.</li></ul>
<h2>Natívny rozbaľovací prvok</h2><details><summary>Rozbaliť podrobnosti</summary><p>Podrobnosti sú rozbalené.</p></details>
<h2>Accordion s JavaScriptom</h2><button id="toggle" aria-expanded="false" aria-controls="panel">Doplňujúce informácie</button>
<div id="panel" hidden><p>Tento obsah sprístupnil JavaScript.</p></div>
<h2 id="tabulka">Skúšobná tabuľka</h2><table><caption>Formáty náhľadu</caption><tr><th scope="col">Formát</th><th scope="col">Skratka</th></tr><tr><th scope="row">HTML</th><td>Alt+Shift+P</td></tr><tr><th scope="row">Markdown</th><td>Alt+Shift+P</td></tr></table>
<h2>Oznámenie stavu</h2><button id="announce">Otestovať oznámenie</button><p id="status" role="status" aria-live="polite" aria-atomic="true"></p>
</main><footer><p>Ponuka Náhľad obsahuje návrat do editora aj presun do obsahu.</p></footer>
<script>let count=0;document.getElementById('toggle').onclick=()=>{const b=document.getElementById('toggle');const p=document.getElementById('panel');p.hidden=!p.hidden;b.setAttribute('aria-expanded',String(!p.hidden));};document.getElementById('announce').onclick=()=>{document.getElementById('status').textContent='Skúšobné oznámenie číslo '+(++count)+'.';};</script></body></html>)HTML";
void Error(HWND hwnd, const wchar_t* text) {
    MessageBoxW(hwnd, text, L"Náhľad — chyba prototypu", MB_OK | MB_ICONERROR);
}
void Resize(const std::shared_ptr<State>& state) {
    if (state->controller && state->window) {
        RECT rect{}; GetClientRect(state->window, &rect); state->controller->put_Bounds(rect);
    }
}
void Start(const std::shared_ptr<State>& state) {
    PWSTR local{};
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) {
        Error(state->window, L"Nepodarilo sa určiť priečinok pre skúšobný profil."); return;
    }
    std::wstring profile = std::wstring(local) + L"\\AccessiblePreview\\ProbeProfile-0.0.3";
    CoTaskMemFree(local);
    // Enable the renderer accessibility tree before creating the document.
    // This enables accessibility only; it does not disable any security feature.
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AdditionalBrowserArguments(L"--force-renderer-accessibility");
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, profile.c_str(), options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [state](HRESULT result, ICoreWebView2Environment* env)->HRESULT {
            if (!state->window) return S_OK;
            if (FAILED(result) || !env) { Error(state->window, L"WebView2 sa nepodarilo spustiť."); return S_OK; }
            return env->CreateCoreWebView2Controller(state->window,
                Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [state](HRESULT result2, ICoreWebView2Controller* controller)->HRESULT {
                    if (!state->window) { if(controller) controller->Close(); return S_OK; }
                    if (FAILED(result2) || !controller) { Error(state->window, L"Nepodarilo sa vytvoriť webový náhľad."); return S_OK; }
                    state->controller = controller;
                    controller->get_CoreWebView2(&state->web);
                    if (!state->web) return E_FAIL;
                    ComPtr<ICoreWebView2Settings> settings;
                    state->web->get_Settings(&settings);
                    settings->put_AreHostObjectsAllowed(FALSE);
                    settings->put_IsWebMessageEnabled(FALSE);
                    settings->put_AreDefaultScriptDialogsEnabled(FALSE);
                    settings->put_AreDevToolsEnabled(FALSE);
                    EventRegistrationToken token{};
                    std::weak_ptr<State> navigationState=state;
                    state->web->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
                        [navigationState](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args)->HRESULT {
                            auto s=navigationState.lock();
                            if(!s) {args->put_Cancel(TRUE);return S_OK;}
                            LPWSTR uri{}; args->get_Uri(&uri);
                            const std::wstring address = uri ? uri : L"";
                            CoTaskMemFree(uri);
                            // NavigateToString may report its internal data URL here,
                            // although the resulting document has about:blank as its URL.
                            // This one-shot exception is ONLY for the compiled-in probe.
                            BOOL user=FALSE, redirected=FALSE;
                            args->get_IsUserInitiated(&user);args->get_IsRedirected(&redirected);
                            const bool initial=s->initialDocumentPending && !user && !redirected &&
                                (address==L"about:blank" || address.rfind(L"data:text/html",0)==0);
                            s->initialDocumentPending=false;
                            const bool anchor=address.rfind(L"about:blank#",0)==0;
                            s->navigationBlocked=!(initial || anchor);
                            if(s->navigationBlocked) args->put_Cancel(TRUE);
                            return S_OK;
                        }).Get(), &token);
                    state->web->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>(
                        [](ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args)->HRESULT { args->put_Handled(TRUE); return S_OK; }).Get(), &token);
                    state->web->add_PermissionRequested(Callback<ICoreWebView2PermissionRequestedEventHandler>(
                        [](ICoreWebView2*, ICoreWebView2PermissionRequestedEventArgs* args)->HRESULT { args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY); return S_OK; }).Get(), &token);
                    std::weak_ptr<State> weak = state;
                    state->web->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
                        [weak](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args)->HRESULT {
                            BOOL success=FALSE; args->get_IsSuccess(&success);
                            if(auto s=weak.lock();s && s->window && s->controller) {
                                if(!success) {
                                    COREWEBVIEW2_WEB_ERROR_STATUS status{};args->get_WebErrorStatus(&status);
                                    std::wstring detail=L"Skúšobný článok sa nepodarilo načítať. Kód WebView2: "+std::to_wstring(static_cast<int>(status));
                                    detail+=s->navigationBlocked?L". Navigáciu zablokoval filter prototypu.":L". Filter prototypu navigáciu nezablokoval.";
                                    Error(s->window,detail.c_str());return S_OK;
                                }
                                s->controller->put_IsVisible(TRUE);
                                s->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
                                s->web->ExecuteScript(L"document.body.focus();", nullptr);
                            }
                            return S_OK;
                        }).Get(), &token);
                    Resize(state);
                    controller->put_IsVisible(TRUE);
                    HRESULT navigation=state->web->NavigateToString(Article);
                    if(FAILED(navigation)) {
                        std::wstring text=L"Spustenie načítania článku zlyhalo. HRESULT: "+std::to_wstring(static_cast<unsigned long>(navigation));
                        Error(state->window,text.c_str());
                    }
                    return navigation;
                }).Get());
        }).Get());
    if (FAILED(hr)) Error(state->window, L"Inicializácia WebView2 zlyhala. Overte prítomnosť runtime.");
}
LRESULT CALLBACK Procedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto raw = reinterpret_cast<State*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if(msg==WM_NCCREATE) {
        raw = static_cast<State*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
        raw->window=hwnd; SetWindowLongPtrW(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(raw));
    }
    auto found=std::find_if(windows.begin(),windows.end(),[raw](const auto& s){return s.get()==raw;});
    auto state=found==windows.end()?std::shared_ptr<State>{}:*found;
    if(state) switch(msg) {
        case WM_SIZE: Resize(state); return 0;
        case WM_SETFOCUS: if(state->controller) state->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC); return 0;
        case WM_COMMAND:
            if(LOWORD(wp)==CloseCommand) { DestroyWindow(hwnd); return 0; }
            if(LOWORD(wp)==FocusContent && state->controller) { state->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC); return 0; }
            break;
        case WM_DESTROY:
            if(state->controller) state->controller->Close();
            state->web.Reset(); state->controller.Reset(); state->window=nullptr;
            if(IsWindow(state->owner)) { SetForegroundWindow(state->owner); if(IsWindow(state->previousFocus)) SetFocus(state->previousFocus); }
            windows.erase(std::remove(windows.begin(),windows.end(),state),windows.end());
            if(state->quit) PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
}
bool OpenPreviewProbe(HINSTANCE module, HWND owner, bool quitOnClose) {
    if(!CheckPreviewRuntime(owner)) return false;
    WNDCLASSW wc{}; wc.hInstance=module; wc.lpfnWndProc=Procedure; wc.lpszClassName=ClassName;
    wc.hCursor=LoadCursorW(nullptr,IDC_ARROW); wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
    if(!RegisterClassW(&wc) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS) return false;
    auto state=std::make_shared<State>(); state->owner=owner; state->previousFocus=GetFocus(); state->quit=quitOnClose;
    windows.push_back(state);
    HMENU menu=CreateMenu(); HMENU submenu=CreatePopupMenu();
    AppendMenuW(submenu,MF_STRING,FocusContent,L"&Prejsť do obsahu");
    AppendMenuW(submenu,MF_STRING,CloseCommand,L"&Zavrieť a vrátiť sa\tAlt+F4");
    AppendMenuW(menu,MF_POPUP,reinterpret_cast<UINT_PTR>(submenu),L"&Náhľad");
    HWND hwnd=CreateWindowExW(0,ClassName,L"Skúška náhľadu — npp-html-markdown-preview",WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,1000,750,owner,menu,module,state.get());
    if(!hwnd) { DestroyMenu(menu); windows.erase(std::remove(windows.begin(),windows.end(),state),windows.end()); return false; }
    ShowWindow(hwnd,SW_SHOW); Start(state); return true;
}
void ClosePreviewProbes() {
    while(!windows.empty()) { auto s=windows.back(); if(s->window) DestroyWindow(s->window); else windows.pop_back(); }
}
