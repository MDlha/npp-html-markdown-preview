#include "SourcePosition.h"
#include <string>

std::optional<size_t> FindSourceLine(std::string_view source,std::string_view text,size_t utf8Offset) {
    if(text.empty() || text.find_first_not_of(" \t\r\n") == std::string_view::npos || utf8Offset>text.size()) return {};
    if(utf8Offset<text.size() && (static_cast<unsigned char>(text[utf8Offset])&0xc0)==0x80) return {};
    // The HTML parser exposes LF even when the source used CRLF or CR.
    std::string normalized;
    if(text.find('\n')!=std::string_view::npos && text.find('\r')==std::string_view::npos && source.find('\r')!=std::string_view::npos) {
        normalized.reserve(source.size());
        for(size_t i=0;i<source.size();++i) {
            if(source[i]!='\r') {normalized.push_back(source[i]);continue;}
            normalized.push_back('\n');
            if(i+1<source.size() && source[i+1]=='\n') ++i;
        }
        source=normalized;
    }
    const auto start=source.find(text);
    if(start==std::string_view::npos || source.find(text,start+1)!=std::string_view::npos) return {};
    size_t line=0;
    for(size_t i=0;i<start+utf8Offset;++i) {
        if(source[i]=='\n' || (source[i]=='\r' && (i+1==source.size() || source[i+1]!='\n'))) ++line;
    }
    return line;
}
