# Licencie a pôvod súčastí / Licenses and component provenance

[Slovenčina](#slovenčina) · [English](#english)

## Slovenčina

Vlastný kód doplnku je dostupný pod **GNU GPL verzie 3 alebo novšej**. Úplný text je v súbore [LICENSE](LICENSE). Súčasti tretích strán sa riadia vlastnými licenciami.

### Zostava 0.8.1

- Zdrojový kód doplnku a postup zostavenia sú pri [značke v0.8.1](https://github.com/MDlha/npp-html-markdown-preview/tree/v0.8.1). Súbor [BUILDING.md](https://github.com/MDlha/npp-html-markdown-preview/blob/v0.8.1/BUILDING.md) uvádza presné revízie a verzie závislostí.
- **MD4C** je pod licenciou MIT. **Hlavičky rozhrania Notepad++** `PluginInterface.h` a `Notepad_plus_msgs.h` uvádzajú GPL verzie 3 alebo novšej. **Hlavičky Scintilly** odkazujú na licenciu Scintilly. Zdroje týchto súčastí sú dostupné v revíziách uvedených v návode na zostavenie.
- **Microsoft WebView2 SDK 1.0.4258.31** má licenciu BSD s tromi podmienkami. DLL doplnku obsahuje staticky pripojený zavádzač `WebView2LoaderStatic.lib` z oficiálneho balíka SDK. **Zdrojový kód presnej verzie tohto zavádzača sa nám nepodarilo dohľadať.** [Licencia SDK](https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.4258.31/License) · [Vyjadrenie tímu WebView2](https://github.com/MicrosoftEdge/WebView2Feedback/issues/846)
- **Microsoft Visual C++**. Zostava používa staticky pripojené knižnice nástrojov MSVC 14.51.36231. Microsoft zverejňuje zdrojový kód štandardnej knižnice C++. **Úplný zodpovedajúci zdrojový kód všetkých staticky pripojených častí Visual C++ pre túto zostavu sa nám nepodarilo dohľadať.** [Dokumentácia ku knižniciam](https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features) · [Zdrojový kód knižnice Microsoft STL](https://github.com/microsoft/STL)

Tento prehľad zaznamenáva pôvod súčastí a dostupnosť ich zdrojov. Posúdenie podmienok šírenia celej DLL 0.8.1 ostáva otvorené.

## English

The plugin's own code is available under the **GNU GPL version 3 or later**. See [LICENSE](LICENSE) for the full text. Third-party components retain their own licenses.

### Version 0.8.1 build

- The plugin source and build instructions are available at [tag v0.8.1](https://github.com/MDlha/npp-html-markdown-preview/tree/v0.8.1). [BUILDING.md](https://github.com/MDlha/npp-html-markdown-preview/blob/v0.8.1/BUILDING.md) identifies the exact dependency revisions and versions.
- **MD4C** is MIT-licensed. The **Notepad++ interface headers** `PluginInterface.h` and `Notepad_plus_msgs.h` state GPL version 3 or later. The **Scintilla headers** refer to the Scintilla license. These components' sources are available at the revisions specified in the build instructions.
- **Microsoft WebView2 SDK 1.0.4258.31** uses a three-clause BSD license. The plugin DLL statically links `WebView2LoaderStatic.lib` from the official SDK package. **We could not locate the source code for this exact version of the loader.** [SDK license](https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.4258.31/License) · [WebView2 team response](https://github.com/MicrosoftEdge/WebView2Feedback/issues/846)
- **Microsoft Visual C++**. The build statically links libraries from MSVC toolset 14.51.36231. Microsoft publishes the C++ standard library sources. **We could not locate complete corresponding source code for all statically linked Visual C++ components in this build.** [Runtime library documentation](https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features) · [Microsoft STL source](https://github.com/microsoft/STL)

This notice records component provenance and source availability. The distribution conditions for the complete 0.8.1 DLL are still under review.

## Licenčné texty / License texts

### MD4C

```text
# The MIT License (MIT)

Copyright © 2016-2026 Martin Mitáš

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the “Software”),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
```

### Scintilla

```text
License for Lexilla, Scintilla, and SciTE

Copyright 1998-2021 by Neil Hodgson <neilh@scintilla.org>

All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.

NEIL HODGSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL NEIL HODGSON BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.
```

### Microsoft WebView2 SDK

[LICENSE.txt z balíka / from package 1.0.4258.31](https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.4258.31/License). Balík SDK obsahuje aj pôvodný `NOTICE.txt`. The SDK package also contains the original `NOTICE.txt`.
