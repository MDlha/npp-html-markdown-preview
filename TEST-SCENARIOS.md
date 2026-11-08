# npp-html-markdown-preview – prehľad skúšok / test scenarios

[Slovenčina](#slovenčina) · [English](#english)

## Slovenčina

Prehľad zahŕňa skúšobné zostavy pre Windows a Notepad++ x64. Pri každom scenári uvádzame, čo sa skúšalo a čo výsledok potvrdzuje.

### Scenáre, ktoré prešli

- **0.6 a 0.6.1 – vykreslenie HTML a Markdownu.** Oba formáty sa načítali v samostatnom skúšobnom náhľade. V 0.6.1 sa potvrdilo aj maximalizované okno.
- **0.7.1 – viac otvorených náhľadov.** Autorka v Notepad++ s NVDA pracovala s tromi náhľadmi naraz. Notepad++ pri tejto skúške nezamrzol a po jeho ukončení sa všetky náhľady zatvorili. Skúšané odkazy v HTML sa otvorili.
- **0.8 – otvorenie a voľba formátu.** Autorka potvrdila skratku **Alt+Shift+P** a prístupný dialóg pri súbore `.txt` vrátane tlačidiel a klávesových skratiek. Rozsiahly HTML dokument vykreslil formuláre, odkazy a rozbaľovacie prvky. Pri skúške ukladania sa súbor nestiahol, čo zodpovedá zámernému blokovaniu sťahovania v náhľade.
- **0.8 – automatické skúšky.** Prešlo 7 zo 7 skúšok zostavy vrátane načítania DLL, rozpoznania prípon, prevodu Markdownu a mapovania na riadok. Skúška načítania DLL overila aj nový názov doplnku. Výmena predošlej inštalácie a obsluha tejto zostavy s NVDA v Notepad++ čakajú na ručné overenie.

### Čo zlyhalo a ako to dopadlo po úprave

- **0.4 → 0.4.1 – prvé načítanie HTML.** WebView2 zablokoval úvodnú navigáciu s chybou 14. Po úprave kontroly očakávanej adresy sa HTML načítalo v samostatnej skúške.
- **0.7 → 0.7.1 – skok z miesta čítania na zdroj.** NVDA neposkytol polohu kurzora pri čítaní bez označenia. Po zmene na označený text autorka potvrdila skok cez **Alt+Shift+S** s Formátovaným výberom NVDA.
- **0.7.1 – návrat do náhľadu.** V pôvodnej zostave sa po skoku na zdroj nedalo vrátiť do náhľadu cez **Alt+Tab**. Po zmene na samostatné okná autorka potvrdila, že návrat funguje.
- **0.8 – zamrznutie NVDA pri dlhom Markdown náhľade.** Pri opakovanej skúške zostal Notepad++ funkčný a NVDA zamrzol počas spracovania fokusu. Doplnok pri veľkých náhľadoch vynechal dodatočný presun fokusu na celé telo stránky. Autorka následne úspešne zopakovala pôvodný scenár. Presná príčina zamrznutia nebola určená.

- **0.8 → 0.8.1 – skok na koniec náhľadu počas nečinnosti.** V 0.8.1 doplnok po úplnom načítaní už znova nenastavuje fokus. Autorka pri skúške s NVDA potvrdila zachovanie miesta čítania v náhľade súboru `.txd`. Prešlo aj všetkých sedem automatických skúšok zostavy.

### Čo sa ešte overuje

Skok na zdroj s JAWS, ohlásenie zablokovaného stiahnutia cez NVDA a návrat fokusu po zatvorení tejto hlášky. Výsledky vyššie sa vzťahujú na opísané scenáre a zostavy.

## English

This summary covers test builds for Windows and Notepad++ x64. Each scenario describes what was tested and what the result confirms.

### Scenarios that passed

- **0.6 and 0.6.1 – HTML and Markdown rendering.** Both formats loaded in the standalone test viewer. Version 0.6.1 also confirmed that the window opened maximized.
- **0.7.1 – multiple open previews.** The project author used three previews at once in Notepad++ with NVDA. Notepad++ remained responsive during this test, and all previews closed when it exited. The HTML links she tried opened.
- **0.8 – opening a preview and selecting a format.** The project author verified **Alt+Shift+P** and the accessible format dialog for a `.txt` file, including its buttons and keyboard shortcuts. A large HTML document rendered forms, links, and expandable elements. The save test did not download a file, consistent with the preview's download restriction.
- **0.8 – automated tests.** All 7 build tests passed, including DLL loading, extension detection, Markdown conversion, and source line mapping. The DLL loading test also checked the new plugin name. Upgrading the previous installation and using this build with NVDA in Notepad++ still require manual testing.

### What failed and how the changes were retested

- **0.4 → 0.4.1 – initial HTML load.** WebView2 blocked the first navigation with error 14. After the expected address check was updated, HTML loaded in the standalone test.
- **0.7 → 0.7.1 – jumping from the reading position to source.** NVDA did not expose the caret position when reading without a selection. After the switch to selected text, the project author verified **Alt+Shift+S** with NVDA Native Selection Mode.
- **0.7.1 – returning to the preview.** In the initial build, **Alt+Tab** could not return to the preview after a source jump. After the previews were given separate windows, the project author verified that switching back worked.
- **0.8 – NVDA freeze with a long Markdown preview.** In the repeat test, Notepad++ remained responsive and NVDA froze while processing focus. The plugin now skips the extra step of focusing the entire page body in large previews. The project author successfully repeated the original scenario. The exact cause of the freeze was not established.

- **0.8 → 0.8.1 – jump to the end of the preview after inactivity.** Version 0.8.1 no longer refocuses the preview when loading completes. The project author confirmed with NVDA that the reading position was preserved in the preview of a `.txd` file. All seven automated build tests also passed.

### Still being verified

The source jump with JAWS, the blocked-download announcement with NVDA, and focus return after dismissing that message. The results above apply to the scenarios and builds described.
