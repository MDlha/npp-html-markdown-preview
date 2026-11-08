# Changelog

## 0.8 — 2026-11-07

- **Alt+Shift+P** opens or refreshes HTML and Markdown previews. The plugin detects known formats automatically and offers a keyboard-accessible choice when it cannot determine the format.
- Adjusted focus handling for large previews that could cause NVDA to freeze.
- The first blocked download in the active preview shows an explanation.

## 0.7.1 — 2026-11-07

- The source jump now uses a selection from the page without copying it to the clipboard. In NVDA browse mode, use Native Selection Mode.
- HTML and Markdown previews use separate Windows windows. This fixed returning via Alt+Tab after a source jump and retaining the window size when switching.

## 0.7 — 2026-11-07

- The initial Alt+Shift+S implementation used the native text caret and a unique match to find the source line. It displayed a message when the position or a match was unavailable.

## 0.6.1 — 2026-11-07

- HTML and Markdown previews open maximized. Refreshing maximizes the existing window.
- The plugin matches each load completion to its navigation. A separate blocked navigation no longer ends the wait for the document.
- Added separate 30-second timeouts for initialization and loading. If WebView2 reports a failure, the preview can be refreshed from the editor.

## 0.6 — 2026-10-18

- The plugin runs without a separate Visual C++ Redistributable or WebView2Loader.dll.
- If the WebView2 Runtime is missing, the plugin shows instructions and a Microsoft link. The runtime is installed separately.

## 0.5 — 2026-10-18

- Added Markdown preview and refresh via Alt+Shift+M. The standalone viewer supports UTF-8 `.md` and `.markdown` files with case-insensitive extensions.
- Markdown is processed locally by MD4C. It renders tables, task lists, strikethrough, footnotes, code, and embedded HTML.

## 0.4.1 — 2026-10-18

- Fixed error 14 on the first load. WebView2 is now allowed to load the expected preview address.

## 0.4 — 2026-10-18

- Added a standalone HTML viewer and the Alt+Shift+H command for previewing the active tab, including unsaved changes.
- Each document has its own window. Closing the preview attempts to restore focus, the caret, and the selection in the editor.
- The preview serves HTML from memory, supports remote HTTPS resources, and opens activated web links in the default browser.
- Blocked frames, downloads, sensitive permissions, and document access to native editor functions.
