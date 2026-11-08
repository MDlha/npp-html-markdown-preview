# npp-html-markdown-preview 0.8.1

**Test build for Notepad++.** The npp-html-markdown-preview plugin renders previews of files written primarily in the HTML and Markdown markup languages in a simple, accessible way. The preview opens from the active tab in a separate, maximized window and includes unsaved changes. The project author designed the features, controls, and accessibility.

### Requirements

- Windows 11 and Notepad++ x64.
- Microsoft Edge WebView2 Evergreen Runtime, which is usually already installed on Windows 11. If it is missing, use the **Evergreen Bootstrapper** with an internet connection or the **Evergreen Standalone Installer x64** offline. Then restart Notepad++. [Download WebView2 from Microsoft](https://developer.microsoft.com/en-us/microsoft-edge/webview2/)
- The plugin runs without Visual Studio or a separate Visual C++ Redistributable.

### Installation or update

1. Close all Notepad++ windows. If you are upgrading from Accessible Preview, move the old `plugins\AccessiblePreview` subfolder outside `plugins` before installing. If you received a ZIP file, extract it.
2. In File Explorer, select **npp-html-markdown-preview.dll** from the release and press **Ctrl+C**.
3. Press **Alt+D**, enter the path to the `plugins` folder, and press **Enter**. A standard installation uses `C:\Program Files\Notepad++\plugins`. A portable installation has its `plugins` folder next to `notepad++.exe`.
4. Open the **npp-html-markdown-preview** subfolder. If it is missing, press **Ctrl+Shift+N**, enter the name, press **Enter**, and open the folder. Press **Ctrl+V** to paste the DLL. When updating an installation with the same name, confirm replacement of the previous DLL. Windows may ask for administrator permission to copy files into Program Files.
5. Start Notepad++ as usual. **npp-html-markdown-preview** appears in the **Plugins** menu.

The standard installation path is `C:\Program Files\Notepad++\plugins\npp-html-markdown-preview\npp-html-markdown-preview.dll`. Keep the DLL and subfolder names as shown. The release includes a checksum in `SHA256SUMS.txt`.

### Controls

- **Alt+Shift+P** opens a preview. Press the same shortcut in the editor to refresh the active tab's existing preview. The plugin menu also offers **Náhľad / obnoviť** (Preview / refresh). You can change the shortcut in the Notepad++ Shortcut Mapper.
- The `.html` and `.htm` extensions select HTML. The `.md` and `.markdown` extensions select Markdown. For an unnamed tab or an unknown extension, the plugin uses HTML if the editor recognizes it. Otherwise, select a format with **Tab** and **Enter**, or cancel with **Escape**. The plugin remembers the choice until you close the tab.
- In the preview, use the arrow keys or your screen reader's quick navigation commands. **Tab** and **Shift+Tab** move focus, **Enter** activates links, and **Space** activates buttons.
- **Alt+Tab** switches between the editor and previews. **Alt+F4** closes the active preview.
- The plugin's menu and messages are in Slovak.

#### Jump to source with NVDA

1. In browse mode, turn on **Native Selection Mode** with **NVDA+Shift+F10** if it is off.
2. Use **Shift+Arrow keys** to select a few letters from a unique heading or paragraph.
3. Press **Alt+Shift+S**. If there is a unique match, the caret moves to the corresponding line in the editor. Press **Alt+Tab** to return to the preview.

If you edit the source, refresh the preview before jumping. Repeated or dynamically generated text may have no unique match. The source jump with NVDA was verified in version 0.7.1. The source jump with JAWS remains unverified.

### Limits and security

- The preview may automatically load images, stylesheets, fonts, and scripts over HTTPS. A script may send data from the article over the internet. Use documents and resources you trust.
- This version does not support local images, stylesheets, or links to local HTML or Markdown files. Web links open in your default browser. The input limit is 32 MiB.
- The preview blocks downloads. The first blocked attempt in the active preview shows a message. Close it with **Enter** or **Escape**. To save a file, open the page in a regular browser.

### Uninstallation

Close Notepad++ and remove the `plugins\npp-html-markdown-preview` folder. Keep WebView2 available for other apps.
