# Zostavenie npp-html-markdown-preview pre Notepad++ x64

Použite Visual Studio 2026 s nástrojmi MSVC x64, Windows SDK a CMake. V priečinku projektu pripravte tieto závislosti:

- [MD4C](https://github.com/mity/md4c/tree/60e8065cfccd5d7ac561115f9a9bfbdf811f5cc9), revízia `60e8065cfccd5d7ac561115f9a9bfbdf811f5cc9`, do `vendor/md4c`.
- [Šablóna doplnku Notepad++](https://github.com/npp-plugins/plugintemplate/tree/abd8c2394936e934753db08ab1684aae1dcf7fd4), revízia `abd8c2394936e934753db08ab1684aae1dcf7fd4`, do `vendor/plugintemplate`.
- [Microsoft.Web.WebView2 1.0.4258.31](https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.4258.31) rozbaľte do `vendor/webview2/sdk`. SHA-256 balíka je `56F7F4B8BF9AEE4B8EFEFBBDD4F67D5F74EBD1B100ED0806DA71BF76AF481AA9`.

V PowerShelli spustite `./scripts/Build.ps1`. Výsledok zostavenia 0.8 je `out/0.8/npp-html-markdown-preview.dll`.
