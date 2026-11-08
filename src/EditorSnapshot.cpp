#include "EditorSnapshot.h"
#include "SourcePosition.h"
#include <vector>
#include <stdexcept>
#include <algorithm>
namespace {
HWND CurrentScintilla(const NppData& npp,int& view) {
    view=-1;SendMessageW(npp._nppHandle,NPPM_GETCURRENTSCINTILLA,0,reinterpret_cast<LPARAM>(&view));
    if(view!=0 && view!=1) throw std::runtime_error("view");
    return view==0?npp._scintillaMainHandle:npp._scintillaSecondHandle;
}
std::string ReadUtf8(HWND sci) {
    std::string text;
    auto length=SendMessageW(sci,SCI_GETLENGTH,0,0);
    if(length<0 || length>32*1024*1024) throw std::runtime_error("size");
    std::vector<char> bytes(static_cast<size_t>(length)+1);
    SendMessageW(sci,SCI_GETTEXT,bytes.size(),reinterpret_cast<LPARAM>(bytes.data()));
    UINT page=static_cast<UINT>(SendMessageW(sci,SCI_GETCODEPAGE,0,0));if(page==0) page=CP_ACP;
    if(length) {
        const DWORD flags=page==CP_UTF8?MB_ERR_INVALID_CHARS:0;
        int count=MultiByteToWideChar(page,flags,bytes.data(),static_cast<int>(length),nullptr,0);
        if(!count) throw std::runtime_error("encoding");
        std::wstring unicode(static_cast<size_t>(count),L'\0');
        if(!MultiByteToWideChar(page,flags,bytes.data(),static_cast<int>(length),unicode.data(),count)) throw std::runtime_error("encoding");
        int size=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,unicode.data(),count,nullptr,0,nullptr,nullptr);
        if(!size) throw std::runtime_error("encoding");text.resize(size);
        if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,unicode.data(),count,text.data(),size,nullptr,nullptr)) throw std::runtime_error("encoding");
    }
    return text;
}
}
HtmlSnapshot CaptureHtmlSnapshot(const NppData& npp) {
    int view{};HWND sci=CurrentScintilla(npp,view);
    HtmlSnapshot snapshot;
    snapshot.buffer=static_cast<uintptr_t>(SendMessageW(npp._nppHandle,NPPM_GETCURRENTBUFFERID,0,0));
    if(!snapshot.buffer) throw std::runtime_error("buffer");
    snapshot.utf8=ReadUtf8(sci);
    // Read only the active editor buffer. Never save it or read stale disk text.
    wchar_t filename[32768]{};
    SendMessageW(npp._nppHandle,NPPM_GETFILENAME,std::size(filename),reinterpret_cast<LPARAM>(filename));
    snapshot.title=filename[0]?filename:L"Nepomenovaný dokument";
    const auto anchor=SendMessageW(sci,SCI_GETANCHOR,0,0), current=SendMessageW(sci,SCI_GETCURRENTPOS,0,0);
    const auto firstLine=SendMessageW(sci,SCI_GETFIRSTVISIBLELINE,0,0);const auto id=snapshot.buffer;
    snapshot.jumpToSource=[npp,id,view,source=snapshot.utf8](std::string_view text,size_t offset)->const wchar_t* {
        try {
            if(!IsWindow(npp._nppHandle)) return L"Zdrojový editor už nie je dostupný.";
            const auto line=FindSourceLine(source,text,offset);
            if(!line) return L"Presný riadok zdroja sa nedá jednoznačne určiť.";
            const auto packed=SendMessageW(npp._nppHandle,NPPM_GETPOSFROMBUFFERID,id,view);
            if(packed<0) return L"Zdrojová karta už nie je otvorená.";
            const auto value=static_cast<unsigned int>(packed);
            const int targetView=static_cast<int>(value>>30), index=static_cast<int>(value&0x3fffffff);
            SendMessageW(npp._nppHandle,NPPM_ACTIVATEDOC,targetView,index);
            int actualView{};
            const HWND target=CurrentScintilla(npp,actualView);
            if(actualView!=targetView || static_cast<uintptr_t>(SendMessageW(npp._nppHandle,NPPM_GETCURRENTBUFFERID,0,0))!=id)
                return L"Zdrojovú kartu sa nepodarilo aktivovať.";
            if(ReadUtf8(target)!=source) return L"Zdroj sa zmenil. Najprv obnovte náhľad skratkou z editora.";
            if(!SetForegroundWindow(npp._nppHandle) && GetForegroundWindow()!=npp._nppHandle)
                return L"Zdrojový editor sa nepodarilo presunúť do popredia.";
            SendMessageW(target,SCI_ENSUREVISIBLE,*line,0);
            SendMessageW(target,SCI_GOTOLINE,*line,0);
            SetFocus(target);
            return nullptr;
        } catch(...) {return L"Zdroj sa nepodarilo bezpečne overiť. Kurzor nebol presunutý.";}
    };
    snapshot.returnToEditor=[npp,id,view,anchor,current,firstLine]() {
        if(!IsWindow(npp._nppHandle)) return;
        auto packed=SendMessageW(npp._nppHandle,NPPM_GETPOSFROMBUFFERID,id,view);SetForegroundWindow(npp._nppHandle);
        if(packed!=-1) {
            const auto value=static_cast<unsigned int>(packed);const int targetView=static_cast<int>(value>>30), index=static_cast<int>(value&0x3fffffff);
            SendMessageW(npp._nppHandle,NPPM_ACTIVATEDOC,targetView,index);
            if(static_cast<uintptr_t>(SendMessageW(npp._nppHandle,NPPM_GETCURRENTBUFFERID,0,0))==id) {
                HWND target=targetView==0?npp._scintillaMainHandle:npp._scintillaSecondHandle;
                const auto limit=SendMessageW(target,SCI_GETLENGTH,0,0);
                SendMessageW(target,SCI_SETSEL,std::clamp<LRESULT>(anchor,0,limit),std::clamp<LRESULT>(current,0,limit));
                SendMessageW(target,SCI_SETFIRSTVISIBLELINE,firstLine,0);SetFocus(target);return;
            }
        }
        int active{};try {SetFocus(CurrentScintilla(npp,active));} catch(...) {}
    };return snapshot;
}
