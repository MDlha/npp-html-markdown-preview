#include "DocumentWindow.h"
#include "RuntimeSupport.h"
#include <objbase.h>
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <wrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Callback;
namespace {
constexpr wchar_t ClassName[]=L"AccessiblePreview.Html.04";
constexpr UINT ReturnCommand=200, FocusCommand=201;
constexpr UINT FailureMessage=WM_APP+1;
constexpr UINT SourceShortcutMessage=WM_APP+2;
constexpr UINT SourceResultMessage=WM_APP+3;
constexpr UINT DownloadBlockedMessage=WM_APP+4;
constexpr UINT_PTR PreviewTimer=1;
constexpr UINT PreviewTimeoutMs=30000;
constexpr size_t MaxBodyFocusBytes=256*1024;
struct Document {
    HWND window{};
    bool fromEditor{};
    HtmlSnapshot snapshot;
    std::wstring url;
    bool loading{}, initializing{}, failed{}, suppressReturn{}, closing{};
    bool downloadNoticePosted{};
    std::optional<UINT64> navigationId;
    const wchar_t* failureText{};
    unsigned long long serial{};
    ULONGLONG deadline{};
    unsigned long long sourceRequest{};
    std::wstring sourceResult;
    ComPtr<ICoreWebView2Environment> environment;
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> web;
};
std::vector<std::shared_ptr<Document>> documents;
void Error(HWND window,const wchar_t* message) {MessageBoxW(window,message,L"Náhľad 0.8",MB_OK|MB_ICONERROR);}
void Fail(const std::shared_ptr<Document>& d,const wchar_t* message) {
    if(d->closing || d->failed) return;
    d->failed=true;d->initializing=false;d->loading=false;d->navigationId.reset();++d->serial;
    KillTimer(d->window,PreviewTimer);
    d->failureText=message;
    PostMessageW(d->window,FailureMessage,0,0);
}
void ReleaseWebView(const std::shared_ptr<Document>& d) {
    if(d->controller) d->controller->Close();
    d->web.Reset();d->controller.Reset();d->environment.Reset();
}
bool ArmTimeout(const std::shared_ptr<Document>& d) {
    d->deadline=GetTickCount64()+PreviewTimeoutMs;
    return SetTimer(d->window,PreviewTimer,PreviewTimeoutMs,nullptr)!=0;
}
bool Elevated() {
    HANDLE token{};if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)) return true;
    TOKEN_ELEVATION elevation{};DWORD size{};
    bool elevated=!GetTokenInformation(token,TokenElevation,&elevation,sizeof(elevation),&size) || elevation.TokenIsElevated;
    CloseHandle(token);return elevated;
}
void Resize(const std::shared_ptr<Document>& d) {
    if(d->controller && d->window) {RECT rect{};GetClientRect(d->window,&rect);d->controller->put_Bounds(rect);}
}
void Focus(const std::shared_ptr<Document>& d) {
    if(d->window && d->controller && !d->failed) {d->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);}
}
void Render(const std::shared_ptr<Document>& d) {
    if(!d->web || d->failed || d->loading) return;
    d->downloadNoticePosted=false;
    ++d->sourceRequest;
    d->navigationId.reset();
    d->loading=true;
    if(!ArmTimeout(d)) {Fail(d,L"Nepodarilo sa nastaviť časový limit náhľadu.");return;}
    HRESULT hr=d->web->Navigate(d->url.c_str());
    if(FAILED(hr)) Fail(d,L"Načítanie náhľadu sa nepodarilo spustiť.");
}
// Require an explicit browser selection. Never use focus or a collapsed caret as a substitute.
constexpr wchar_t SourceSelectionScript[]=LR"JS((()=>{
    const s=window.getSelection();
    if(!s || s.isCollapsed || s.rangeCount!==1 || !document.body) return '';
    const range=s.getRangeAt(0);
    if(!document.body.contains(range.startContainer) || !document.body.contains(range.endContainer)) return '';
    const walker=document.createTreeWalker(document.body,NodeFilter.SHOW_TEXT);
    let n=null, o=0, count=0;
    while(walker.nextNode()) {
        if(++count>50000) return '';
        const node=walker.currentNode, text=node.nodeValue;
        if(!range.intersectsNode(node)) continue;
        const start=range.startContainer===node?range.startOffset:0;
        const end=range.endContainer===node?range.endOffset:text.length;
        if(start>=end || !text.slice(start,end).trim()) continue;
        n=node; o=start; break;
    }
    if(!n || n.parentElement.closest('script,style,textarea,[contenteditable]')) return '';
    const t=n.nodeValue;
    if(t.length>8192) return '';
    if(o<t.length && (t.charCodeAt(o)&0xfc00)===0xdc00) return '';
    walker.currentNode=document.body;
    count=0;
    while(walker.nextNode()) {
        if(++count>50000 || (walker.currentNode!==n && walker.currentNode.nodeValue===t)) return '';
    }
    const encoder=new TextEncoder();
    let hex=''; for(const byte of encoder.encode(t)) hex+=byte.toString(16).padStart(2,'0');
    return encoder.encode(t.slice(0,o)).length+':'+hex;
})())JS";

void SourceNotice(HWND window,const wchar_t* message) {
    MessageBoxW(window,message,L"Prejsť na zdroj",MB_OK|MB_ICONINFORMATION);
}
int HexDigit(wchar_t c) {
    if(c>=L'0' && c<=L'9') return c-L'0';
    if(c>=L'a' && c<=L'f') return c-L'a'+10;
    return -1;
}
void JumpFromResult(const std::shared_ptr<Document>& d) {
    auto result=std::move(d->sourceResult);
    d->sourceResult.clear();
    std::wstring_view data(result);
    const auto sep=data.find(L':');
    if(data.size()<5 || data.front()!=L'"' || data.back()!=L'"' || sep<2 || sep>6 || (data.size()-sep-2)%2) {
        SourceNotice(d->window,L"Náhľad nemá použiteľný výber textu. Označte krátky úsek v náhľade a stlačte Alt+Shift+S. Pri NVDA v režime prehliadania zapnite Formátovaný výber (NVDA+Shift+F10), ak ešte nie je zapnutý. Text netreba kopírovať.");return;
    }
    size_t offset=0;
    for(size_t i=1;i<sep;++i) {
        if(data[i]<L'0' || data[i]>L'9') {SourceNotice(d->window,L"Pozíciu kurzora sa nepodarilo overiť.");return;}
        offset=offset*10+data[i]-L'0';
    }
    std::string text;
    text.reserve((data.size()-sep-2)/2);
    for(size_t i=sep+1;i+1<data.size()-1;i+=2) {
        int high=HexDigit(data[i]),low=HexDigit(data[i+1]);
        if(high<0 || low<0) {SourceNotice(d->window,L"Text pri kurzore sa nepodarilo overiť.");return;}
        text.push_back(static_cast<char>(16*high+low));
    }
    if(const auto* error=d->snapshot.jumpToSource(text,offset)) SourceNotice(d->window,error);
}
void RequestSource(const std::shared_ptr<Document>& d) {
    if(!d->snapshot.jumpToSource) {SourceNotice(d->window,L"Presun na zdroj je dostupný iba z náhľadu otvoreného v Notepad++.");return;}
    if(d->loading || d->initializing || !d->web) {SourceNotice(d->window,L"Náhľad ešte nie je pripravený.");return;}
    std::weak_ptr<Document> weak=d;
    const auto request=++d->sourceRequest, generation=d->serial;
    auto handler=Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
        [weak,request,generation](HRESULT result,LPCWSTR value)->HRESULT {
            auto s=weak.lock();
            if(!s || !s->window || s->closing || s->failed || generation!=s->serial || request!=s->sourceRequest) return S_OK;
            try {
                s->sourceResult.clear();
                if(SUCCEEDED(result) && value) {
                    const std::wstring_view data(value);
                    if(data.size()<=65550) s->sourceResult.assign(data);
                }
                PostMessageW(s->window,SourceResultMessage,static_cast<WPARAM>(request),0);
                return S_OK;
            } catch(...) {return E_OUTOFMEMORY;}
        });
    if(!handler || FAILED(d->web->ExecuteScript(SourceSelectionScript,handler.Get())))
        SourceNotice(d->window,L"Označenie textu v náhľade sa nepodarilo získať.");
}
// Only HTTP(S) links explicitly activated by the user leave the preview.
// No shell command is assembled from document text.
bool WebLink(const std::wstring& address) {
    if(address.size()>8192 || address.find_first_of(L"\r\n\t")!=std::wstring::npos) return false;
    ComPtr<IUri> uri;
    if(FAILED(CreateUri(address.c_str(),Uri_CREATE_CANONICALIZE,0,&uri))) return false;
    DWORD scheme{};uri->GetScheme(&scheme);
    return scheme==URL_SCHEME_HTTPS || scheme==URL_SCHEME_HTTP;
}
void ExternalLink(const std::shared_ptr<Document>& d,const std::wstring& address) {
    if(!WebLink(address)) {Error(d->window,L"Táto verzia otvára iba webové odkazy HTTP a HTTPS. Miestne a e-mailové odkazy zatiaľ nie sú podporované.");return;}
    SHELLEXECUTEINFOW info{sizeof(info)};info.fMask=SEE_MASK_FLAG_NO_UI;
    info.hwnd=d->window;info.lpVerb=L"open";info.lpFile=address.c_str();info.nShow=SW_SHOWNORMAL;
    if(!ShellExecuteExW(&info)) Error(d->window,L"Odkaz sa nepodarilo otvoriť v predvolenom prehliadači.");
}
HRESULT Respond(const std::shared_ptr<Document>& d, ICoreWebView2WebResourceRequestedEventArgs* args,
                const std::string& content,int code,const wchar_t* reason,const wchar_t* headers) {
    ComPtr<IStream> stream;
    if(!content.empty()) stream.Attach(SHCreateMemStream(reinterpret_cast<const BYTE*>(content.data()),static_cast<UINT>(content.size())));
    if(!content.empty() && !stream) return E_OUTOFMEMORY;
    ComPtr<ICoreWebView2WebResourceResponse> response;
    HRESULT hr=d->environment->CreateWebResourceResponse(stream.Get(),code,reason,headers,&response);
    if(FAILED(hr)) return hr;
    return args->put_Response(response.Get());
}
bool Configure(const std::shared_ptr<Document>& d) {
    ComPtr<ICoreWebView2Settings> settings;
    if(FAILED(d->web->get_Settings(&settings)) || !settings ||
       FAILED(settings->put_AreHostObjectsAllowed(FALSE)) || FAILED(settings->put_IsWebMessageEnabled(FALSE)) ||
       FAILED(settings->put_AreDefaultScriptDialogsEnabled(FALSE)) || FAILED(settings->put_AreDevToolsEnabled(FALSE))) return false;
    ComPtr<ICoreWebView2Settings4> settings4;
    if(SUCCEEDED(settings.As(&settings4))) {settings4->put_IsPasswordAutosaveEnabled(FALSE);settings4->put_IsGeneralAutofillEnabled(FALSE);}
    std::weak_ptr<Document> weak=d;EventRegistrationToken token{};
    if(FAILED(d->controller->add_AcceleratorKeyPressed(Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>(
        [weak](ICoreWebView2Controller* sender,ICoreWebView2AcceleratorKeyPressedEventArgs* args)->HRESULT {
            auto s=weak.lock();
            if(!s || s->closing || s->failed || !s->window || sender!=s->controller.Get()) return S_OK;
            COREWEBVIEW2_KEY_EVENT_KIND kind{};
            UINT key{};
            if(FAILED(args->get_KeyEventKind(&kind)) ||
               (kind!=COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN && kind!=COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN) ||
               FAILED(args->get_VirtualKey(&key)) || key!='S' ||
               !(GetKeyState(VK_MENU)&0x8000) || !(GetKeyState(VK_SHIFT)&0x8000) ||
               (GetKeyState(VK_CONTROL)&0x8000) || (GetKeyState(VK_LWIN)&0x8000) ||
               (GetKeyState(VK_RWIN)&0x8000)) return S_OK;
            if(FAILED(args->put_Handled(TRUE))) return S_OK;
            COREWEBVIEW2_PHYSICAL_KEY_STATUS status{};
            if(SUCCEEDED(args->get_PhysicalKeyStatus(&status)) && !status.WasKeyDown)
                PostMessageW(s->window,SourceShortcutMessage,0,0);
            return S_OK;
        }).Get(),&token))) return false;
    if(FAILED(d->web->AddWebResourceRequestedFilter(L"*",COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL))) return false;
    if(FAILED(d->web->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>(
        [weak](ICoreWebView2* sender,ICoreWebView2WebResourceRequestedEventArgs* args)->HRESULT {
            auto s=weak.lock();if(!s || s->closing || s->failed || sender!=s->web.Get()) return E_ABORT;
            ComPtr<ICoreWebView2WebResourceRequest> request;
            if(FAILED(args->get_Request(&request)) || !request) return E_FAIL;
            LPWSTR raw{};request->get_Uri(&raw);std::wstring uri=raw?raw:L"";CoTaskMemFree(raw);
            LPWSTR method{};request->get_Method(&method);bool get=method && wcscmp(method,L"GET")==0;CoTaskMemFree(method);
            COREWEBVIEW2_WEB_RESOURCE_CONTEXT context{};args->get_ResourceContext(&context);
            if(uri==s->url && get && context==COREWEBVIEW2_WEB_RESOURCE_CONTEXT_DOCUMENT) {
                // Header CSP cannot be relaxed by a meta tag in the author's HTML.
                return Respond(s,args,s->snapshot.utf8,200,L"OK",
                    L"Content-Type: text/html; charset=utf-8\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nReferrer-Policy: no-referrer\r\n"
                    L"Content-Security-Policy: default-src 'none'; img-src https: data:; style-src https: 'unsafe-inline'; font-src https: data:; script-src https: 'unsafe-inline'; connect-src 'none'; frame-src 'none'; object-src 'none'; worker-src 'none'; form-action 'none'; base-uri 'none'\r\n");
            }
            // v0.4 deliberately has no disk mapping. Unknown synthetic-host URLs
            // cannot fall through to the network. Remote assets are HTTPS only.
            const bool asset=context==COREWEBVIEW2_WEB_RESOURCE_CONTEXT_IMAGE || context==COREWEBVIEW2_WEB_RESOURCE_CONTEXT_STYLESHEET ||
                context==COREWEBVIEW2_WEB_RESOURCE_CONTEXT_FONT || context==COREWEBVIEW2_WEB_RESOURCE_CONTEXT_SCRIPT;
            if(get && asset && uri.rfind(L"https://",0)==0 && uri.rfind(L"https://preview-",0)!=0) return S_OK;
            return Respond(s,args,"",403,L"Blocked",L"Cache-Control: no-store\r\n");
        }).Get(),&token))) return false;
    if(FAILED(d->web->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
        [weak](ICoreWebView2* sender,ICoreWebView2NavigationStartingEventArgs* args)->HRESULT {
            auto s=weak.lock();if(!s || s->closing || s->failed || sender!=s->web.Get()) {args->put_Cancel(TRUE);return S_OK;}
            LPWSTR raw{};args->get_Uri(&raw);std::wstring uri=raw?raw:L"";CoTaskMemFree(raw);
            BOOL user=FALSE,redirect=FALSE;args->get_IsUserInitiated(&user);args->get_IsRedirected(&redirect);
            // WebView2 can mark a host-API Navigate as user initiated too.
            // The exact generated URL + pending host load is the authority here.
            UINT64 id{};
            if(uri==s->url && s->loading && !redirect && !s->navigationId) {
                if(SUCCEEDED(args->get_NavigationId(&id))) {s->navigationId=id;return S_OK;}
                Fail(s,L"Nepodarilo sa identifikovať načítanie náhľadu.");
            }
            if(uri.rfind(s->url+L"#",0)==0 && !redirect) return S_OK;
            args->put_Cancel(TRUE);
            if(user && !redirect && !s->loading) ExternalLink(s,uri);
            return S_OK;
        }).Get(),&token))) return false;
    if(FAILED(d->web->add_FrameNavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
        [](ICoreWebView2*,ICoreWebView2NavigationStartingEventArgs* args)->HRESULT {args->put_Cancel(TRUE);return S_OK;}).Get(),&token))) return false;
    if(FAILED(d->web->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>(
        [weak](ICoreWebView2* sender,ICoreWebView2NewWindowRequestedEventArgs* args)->HRESULT {
            args->put_Handled(TRUE);BOOL user=FALSE;args->get_IsUserInitiated(&user);
            if(auto s=weak.lock();s && user && !s->closing && !s->failed && !s->loading && sender==s->web.Get()) {
                LPWSTR raw{};args->get_Uri(&raw);std::wstring uri=raw?raw:L"";CoTaskMemFree(raw);ExternalLink(s,uri);
            }return S_OK;
        }).Get(),&token))) return false;
    if(FAILED(d->web->add_PermissionRequested(Callback<ICoreWebView2PermissionRequestedEventHandler>(
        [](ICoreWebView2*,ICoreWebView2PermissionRequestedEventArgs* args)->HRESULT {args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);return S_OK;}).Get(),&token))) return false;
    ComPtr<ICoreWebView2_4> web4;
    if(FAILED(d->web.As(&web4)) || FAILED(web4->add_DownloadStarting(Callback<ICoreWebView2DownloadStartingEventHandler>(
        [weak](ICoreWebView2* sender,ICoreWebView2DownloadStartingEventArgs* args)->HRESULT {
            const HRESULT canceled=args->put_Cancel(TRUE);
            if(FAILED(canceled)) return canceled;
            auto s=weak.lock();
            // A document can start downloads itself. Announce at most once per load and never from the background.
            if(s && !s->closing && !s->failed && !s->loading && sender==s->web.Get() &&
               !s->downloadNoticePosted && GetForegroundWindow()==s->window) {
                s->downloadNoticePosted=PostMessageW(s->window,DownloadBlockedMessage,0,0)!=FALSE;
            }
            return S_OK;
        }).Get(),&token))) return false;
    if(FAILED(d->web->add_ProcessFailed(Callback<ICoreWebView2ProcessFailedEventHandler>(
        [weak](ICoreWebView2* sender,ICoreWebView2ProcessFailedEventArgs* args)->HRESULT {
            auto s=weak.lock();if(!s || sender!=s->web.Get()) return S_OK;
            COREWEBVIEW2_PROCESS_FAILED_KIND kind{};
            if(FAILED(args->get_ProcessFailedKind(&kind)) || kind==COREWEBVIEW2_PROCESS_FAILED_KIND_BROWSER_PROCESS_EXITED ||
               kind==COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_EXITED || kind==COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_UNRESPONSIVE)
                Fail(s,L"Vykresľovanie náhľadu zlyhalo alebo neodpovedá.");
            return S_OK;
        }).Get(),&token))) return false;
    if(FAILED(d->web->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
        [weak](ICoreWebView2* sender,ICoreWebView2NavigationCompletedEventArgs* args)->HRESULT {
            auto s=weak.lock();if(!s || s->closing || s->failed || !s->window || sender!=s->web.Get()) return S_OK;
            UINT64 id{};
            if(!s->navigationId || FAILED(args->get_NavigationId(&id)) || id!=*s->navigationId) return S_OK;
            s->loading=false;s->navigationId.reset();KillTimer(s->window,PreviewTimer);
            BOOL ok=FALSE;args->get_IsSuccess(&ok);
            if(s->snapshot.loadResult) {
                COREWEBVIEW2_WEB_ERROR_STATUS code{};args->get_WebErrorStatus(&code);
                s->snapshot.loadResult(ok!=FALSE,static_cast<int>(code),IsZoomed(s->window)!=FALSE);return S_OK;
            }
            if(!ok) {
                Fail(s,L"Dokument sa nepodarilo načítať.");
            } else {
                s->controller->put_IsVisible(TRUE);
                if(GetForegroundWindow()==s->window) {
                    Focus(s);
                    // Focusing the entire body of a large page can give a screen reader a very large text range.
                    if(s->snapshot.utf8.size()<=MaxBodyFocusBytes)
                        s->web->ExecuteScript(L"if(document.body){document.body.setAttribute('tabindex','-1');document.body.focus();}",nullptr);
                }
            }return S_OK;
        }).Get(),&token))) return false;
    Resize(d);return SUCCEEDED(d->controller->put_IsVisible(TRUE));
}
void Start(const std::shared_ptr<Document>& d) {
    ReleaseWebView(d);
    d->failed=false;d->failureText=nullptr;d->initializing=true;
    const auto generation=++d->serial;
    if(!ArmTimeout(d)) {Fail(d,L"Nepodarilo sa nastaviť časový limit náhľadu.");return;}
    PWSTR root{};if(FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&root))) {Fail(d,L"Nie je dostupný priečinok pre profil náhľadu.");return;}
    std::wstring profile=std::wstring(root)+L"\\AccessiblePreview\\Html-0.6";CoTaskMemFree(root);
    auto options=Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    if(!options || FAILED(options->put_AdditionalBrowserArguments(L"--force-renderer-accessibility")) ||
       FAILED(options->put_AllowSingleSignOnUsingOSPrimaryAccount(FALSE))) {Fail(d,L"Nepodarilo sa nastaviť WebView2.");return;}
    std::weak_ptr<Document> weak=d;
    HRESULT hr=CreateCoreWebView2EnvironmentWithOptions(nullptr,profile.c_str(),options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([weak,generation](HRESULT result,ICoreWebView2Environment* env)->HRESULT {
            auto s=weak.lock();if(!s || s->closing || generation!=s->serial) return S_OK;
            if(FAILED(result) || !env) {Fail(s,L"WebView2 sa nepodarilo spustiť.");return S_OK;}
            s->environment=env;
            const HRESULT create=env->CreateCoreWebView2Controller(s->window,Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [weak,generation](HRESULT result2,ICoreWebView2Controller* controller)->HRESULT {
                    auto t=weak.lock();if(!t || t->closing || generation!=t->serial) {if(controller) controller->Close();return S_OK;}
                    if(FAILED(result2) || !controller) {Fail(t,L"Nepodarilo sa vytvoriť okno WebView2.");return S_OK;}
                    t->controller=controller;
                    if(FAILED(controller->get_CoreWebView2(&t->web)) || !t->web || !Configure(t)) {
                        Fail(t,L"Nepodarilo sa pripraviť WebView2 s potrebnými ochranami.");return S_OK;
                    }
                    t->initializing=false;Render(t);return S_OK;
                }).Get());
            if(FAILED(create)) Fail(s,L"Vytváranie okna WebView2 sa nepodarilo spustiť.");
            return S_OK;
        }).Get());
    if(FAILED(hr)) Fail(d,L"Inicializácia náhľadu zlyhala.");
}
LRESULT CALLBACK Procedure(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    if(msg==WM_NCCREATE) SetWindowLongPtrW(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams));
    auto raw=reinterpret_cast<Document*>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
    auto it=std::find_if(documents.begin(),documents.end(),[raw](auto& d){return d.get()==raw;});
    auto d=it!=documents.end()?*it:std::shared_ptr<Document>{};
    if(d) switch(msg) {
        case WM_SIZE:Resize(d);return 0;
        case WM_MOVE:if(d->controller) d->controller->NotifyParentWindowPositionChanged();break;
        case WM_SETFOCUS:Focus(d);return 0;
        case SourceShortcutMessage:
            if(!d->closing && !d->failed && GetForegroundWindow()==hwnd)
                RequestSource(d);
            return 0;
        case SourceResultMessage:
            if(!d->closing && !d->failed && !d->loading && wp==d->sourceRequest && d->snapshot.jumpToSource && GetForegroundWindow()==hwnd) {
                try {JumpFromResult(d);} catch(...) {SourceNotice(hwnd,L"Presný riadok zdroja sa nepodarilo zistiť.");}
            }
            return 0;
        case DownloadBlockedMessage:
            if(d->downloadNoticePosted && !d->closing && !d->failed && !d->loading) {
                if(GetForegroundWindow()==hwnd)
                    MessageBoxW(hwnd,L"V bežnom prehliadači by sa spustilo sťahovanie. Náhľad sťahovanie blokuje, preto sa súbor neuložil.",
                                L"Sťahovanie v náhľade",MB_OK|MB_ICONINFORMATION);
                else
                    d->downloadNoticePosted=false;
            }
            return 0;
        case WM_TIMER:
            if(wp==PreviewTimer) {
                if((d->loading || d->initializing) && GetTickCount64()>=d->deadline) Fail(d,L"Náhľad sa nenačítal v časovom limite.");
                return 0;
            }break;
        case FailureMessage:
            if(d->failed && d->failureText) {
                const wchar_t* message=d->failureText;d->failureText=nullptr;
                ReleaseWebView(d);
                if(d->snapshot.loadResult) {d->snapshot.loadResult(false,-1,IsZoomed(hwnd)!=FALSE);return 0;}
                auto detail=std::wstring(message)+L"\n\n"+
                    (d->fromEditor?L"Náhľad zatvoríte cez Alt+F4 alebo sa vrátite do editora a zopakujete skratku.":L"Náhľad zatvoríte cez Alt+F4 a môžete ho spustiť znova.");
                Error(hwnd,detail.c_str());
            }return 0;
        case WM_COMMAND:
            if(LOWORD(wp)==ReturnCommand) {DestroyWindow(hwnd);return 0;}
            if(LOWORD(wp)==FocusCommand) {Focus(d);return 0;}break;
        case WM_DESTROY: {
            d->closing=true;KillTimer(hwnd,PreviewTimer);ReleaseWebView(d);d->window=nullptr;
            documents.erase(std::remove(documents.begin(),documents.end(),d),documents.end());
            if(!d->suppressReturn && d->snapshot.returnToEditor) d->snapshot.returnToEditor();
            return 0;
        }
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
}
bool ShowHtmlSnapshot(HINSTANCE module,HWND owner,HtmlSnapshot snapshot) {
    if(!CheckPreviewRuntime(owner)) return false;
    if(Elevated()) {Error(owner,L"Z bezpečnostných dôvodov náhľad nespúšťajte v Notepad++ otvorenom ako správca.");return false;}
    auto existing=std::find_if(documents.begin(),documents.end(),[&snapshot](auto& d){return d->snapshot.buffer==snapshot.buffer;});
    if(existing!=documents.end()) {
        auto d=*existing;
        if(d->loading || d->initializing) {
            ShowWindow(d->window,SW_SHOWMAXIMIZED);SetForegroundWindow(d->window);
            Error(d->window,L"Náhľad sa ešte načítava. Skúste skratku po dokončení znova.");return false;
        }
        d->snapshot=std::move(snapshot);
        auto title=std::wstring(d->snapshot.markdown?L"Markdown náhľad 0.8 — ":L"HTML náhľad 0.8 — ")+d->snapshot.title;SetWindowTextW(d->window,title.c_str());
        ShowWindow(d->window,SW_SHOWMAXIMIZED);SetForegroundWindow(d->window);
        if(d->failed || !d->web) Start(d);else {Render(d);Focus(d);}
        return true;
    }
    WNDCLASSW wc{};wc.hInstance=module;wc.lpfnWndProc=Procedure;wc.lpszClassName=ClassName;
    wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
    if(!RegisterClassW(&wc) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS) {Error(owner,L"Nepodarilo sa pripraviť okno náhľadu.");return false;}
    GUID guid{};if(FAILED(CoCreateGuid(&guid))) {Error(owner,L"Nepodarilo sa vytvoriť adresu náhľadu.");return false;}
    wchar_t id[40]{};StringFromGUID2(guid,id,40);
    auto d=std::make_shared<Document>();d->fromEditor=owner!=nullptr;d->snapshot=std::move(snapshot);
    d->url=L"https://preview-"+std::wstring(id+1,36)+L".invalid/index.html";
    CharLowerBuffW(d->url.data(),static_cast<DWORD>(d->url.size()));
    documents.push_back(d);
    HMENU menu=CreateMenu(),sub=CreatePopupMenu();
    AppendMenuW(sub,MF_STRING,FocusCommand,L"&Prejsť do obsahu");
    AppendMenuW(sub,MF_STRING,ReturnCommand,L"&Zavrieť a vrátiť sa do editora\tAlt+F4");
    AppendMenuW(menu,MF_POPUP,reinterpret_cast<UINT_PTR>(sub),L"&Náhľad");
    auto title=std::wstring(d->snapshot.markdown?L"Markdown náhľad 0.8 — ":L"HTML náhľad 0.8 — ")+d->snapshot.title;
    // An owned window can share the editor's Alt+Tab entry and follows its window state.
    // The plugin already closes previews when the source tab or editor closes.
    d->window=CreateWindowExW(WS_EX_APPWINDOW,ClassName,title.c_str(),WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,CW_USEDEFAULT,1000,750,nullptr,menu,module,d.get());
    if(!d->window) {DestroyMenu(menu);documents.erase(std::remove(documents.begin(),documents.end(),d),documents.end());Error(owner,L"Nepodarilo sa vytvoriť okno náhľadu.");return false;}
    ShowWindow(d->window,SW_SHOWMAXIMIZED);SetForegroundWindow(d->window);Start(d);return true;
}
void ForgetHtmlSnapshot(uintptr_t buffer) {
    auto it=std::find_if(documents.begin(),documents.end(),[buffer](auto& d){return d->snapshot.buffer==buffer;});
    if(it!=documents.end()) {auto d=*it;d->suppressReturn=true;DestroyWindow(d->window);}
}
void CloseHtmlSnapshots() {
    while(!documents.empty()) {auto d=documents.back();d->suppressReturn=true;if(d->window) DestroyWindow(d->window);else documents.pop_back();}
}
