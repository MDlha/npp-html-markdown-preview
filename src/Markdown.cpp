#include "Markdown.h"
#include <md4c-html.h>
#include <stdexcept>
#include <exception>
namespace {
struct Output {std::string text;std::exception_ptr error;};
void Append(const MD_CHAR* data,MD_SIZE size,void* context) noexcept {
    auto& output=*static_cast<Output*>(context);
    if(output.error) return;
    try {
        if(output.text.size()+size>128*1024*1024) throw std::runtime_error("Markdown output too large");
        output.text.append(data,size);
    } catch(...) {output.error=std::current_exception();}
}
}
std::string RenderMarkdown(std::string_view source) {
    if(source.size()>32*1024*1024) throw std::runtime_error("Markdown input too large");
    Output output;
    int result=md_html(source.data(),static_cast<MD_SIZE>(source.size()),Append,&output,
        MD_DIALECT_GITHUB,MD_HTML_FLAG_SKIP_UTF8_BOM);
    if(output.error) std::rethrow_exception(output.error);
    if(result) throw std::runtime_error("Markdown conversion failed");
    return std::string(R"(<!doctype html><html lang="sk"><head><meta charset="utf-8"><title>Markdown náhľad</title><style>
body{font:1.125rem system-ui;line-height:1.65;max-width:70rem;margin:2rem auto;padding:0 1rem;color:#202020;background:#fff}img{max-width:100%;height:auto}pre{overflow:auto;padding:1rem;background:#eee}code{font-family:Consolas,monospace}table{border-collapse:collapse;max-width:100%}td,th{border:1px solid;padding:.4rem .7rem}blockquote{border-left:.25rem solid;padding-left:1rem}a{color:#0645ad}:focus-visible{outline:3px solid Highlight;outline-offset:2px}@media(prefers-color-scheme:dark){body{color:#eee;background:#181818}a{color:#9dcaff}pre{background:#282828}}@media(forced-colors:active){body,pre{color:CanvasText;background:Canvas}a{color:LinkText}}
</style></head><body>)")+output.text+"</body></html>";
}
