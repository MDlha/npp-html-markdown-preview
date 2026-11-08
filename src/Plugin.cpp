#include <windows.h>
#include <objbase.h>
#include <unordered_map>
#include "PluginInterface.h"
#include "PreviewWindow.h"
#include "EditorSnapshot.h"
#include "Markdown.h"
#include "PreviewFormat.h"
#include "Resource.h"
namespace {
HINSTANCE module{};
NppData npp{};
FuncItem commands[2];
ShortcutKey previewShortcut{false,true,true,'P'};
std::unordered_map<uintptr_t, PreviewFormat> chosenFormats;
bool initialized{};
bool comInitialized{};
INT_PTR CALLBACK FormatDialog(HWND dialog, UINT message, WPARAM wp, LPARAM) {
    if (message == WM_INITDIALOG) return TRUE;
    if (message != WM_COMMAND) return FALSE;
    const auto choice = LOWORD(wp);
    if (choice != IDYES && choice != IDNO && choice != IDCANCEL) return FALSE;
    EndDialog(dialog, choice);
    return TRUE;
}

std::optional<PreviewFormat> SelectFormat(const HtmlSnapshot& snapshot) {
    if (auto format = FormatFromFilename(snapshot.title)) return format;
    const auto language = SendMessageW(npp._nppHandle, NPPM_GETBUFFERLANGTYPE, snapshot.buffer, 0);
    if (language == L_HTML) return PreviewFormat::Html;
    const auto saved = chosenFormats.find(snapshot.buffer);
    if (saved != chosenFormats.end()) return saved->second;

    const auto choice = DialogBoxParamW(module, MAKEINTRESOURCEW(IDD_PREVIEW_FORMAT),
                                        npp._nppHandle, FormatDialog, 0);
    if (choice == -1) {
        MessageBoxW(npp._nppHandle,L"Výber formátu sa nepodarilo otvoriť.",L"Náhľad 0.8.1",MB_OK|MB_ICONERROR);
    }
    if (choice != IDYES && choice != IDNO) return std::nullopt;
    if (SendMessageW(npp._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0) != static_cast<LRESULT>(snapshot.buffer))
        return std::nullopt;
    const auto format = choice == IDYES ? PreviewFormat::Html : PreviewFormat::Markdown;
    chosenFormats.emplace(snapshot.buffer, format);
    return format;
}

void Preview() {
    if(!comInitialized) {
        HRESULT hr=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
        if(FAILED(hr)) {MessageBoxW(npp._nppHandle,L"Nepodarilo sa inicializovať COM.",L"Náhľad 0.8.1",MB_OK|MB_ICONERROR);return;}
        comInitialized=true;
    }
    try {
        auto snapshot=CaptureHtmlSnapshot(npp);
        const auto format=SelectFormat(snapshot);
        if(!format) return;
        snapshot.markdown=*format==PreviewFormat::Markdown;
        if(snapshot.markdown) snapshot.utf8=RenderMarkdown(snapshot.utf8);
        ShowHtmlSnapshot(module,npp._nppHandle,std::move(snapshot));
    } catch(...) {
        MessageBoxW(npp._nppHandle,L"Obsah editora sa nepodarilo načítať. Limit tejto verzie je 32 MiB; dokument musí mať platné textové kódovanie.",L"Náhľad 0.8.1",MB_OK|MB_ICONERROR);
    }
}
void Probe() {
    if(!comInitialized) {
        HRESULT hr=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
        if(FAILED(hr)) {MessageBoxW(npp._nppHandle,L"Nepodarilo sa inicializovať COM pre náhľad.",L"Náhľad",MB_OK|MB_ICONERROR);return;}
        comInitialized=true;
    }
    if(!OpenPreviewProbe(module,npp._nppHandle)) MessageBoxW(npp._nppHandle,L"Nepodarilo sa otvoriť skúšobné okno.",L"Náhľad",MB_OK|MB_ICONERROR);
}
}
BOOL APIENTRY DllMain(HINSTANCE instance,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH) {module=instance;DisableThreadLibraryCalls(instance);} return TRUE;
}
extern "C" __declspec(dllexport) void setInfo(NppData data) { npp=data; }
extern "C" __declspec(dllexport) const wchar_t* getName() { return L"npp-html-markdown-preview"; }
extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count) {
    if(!initialized) {
        lstrcpyW(commands[0]._itemName,L"Náhľad / obnoviť");commands[0]._pFunc=Preview;commands[0]._pShKey=&previewShortcut;
        lstrcpyW(commands[1]._itemName,L"Skúška prístupnosti náhľadu");commands[1]._pFunc=Probe;initialized=true;
    }
    *count=2; return commands;
}
extern "C" __declspec(dllexport) void beNotified(SCNotification* note) {
    if(!note) return;
    if(note->nmhdr.code==NPPN_FILECLOSED) {
        chosenFormats.erase(note->nmhdr.idFrom);
        ForgetHtmlSnapshot(note->nmhdr.idFrom);
    }
    if(note->nmhdr.code==NPPN_BEFORESHUTDOWN) {CloseHtmlSnapshots();ClosePreviewProbes();}
    if(note->nmhdr.code==NPPN_SHUTDOWN) {CloseHtmlSnapshots();ClosePreviewProbes(); if(comInitialized) {CoUninitialize();comInitialized=false;}}
}
extern "C" __declspec(dllexport) LRESULT messageProc(UINT,WPARAM,LPARAM) { return TRUE; }
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
