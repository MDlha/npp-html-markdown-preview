# História zmien

## 0.8 — 2026-11-07

- **Alt+Shift+P** otvorí alebo obnoví náhľad HTML aj Markdownu. Pri známom formáte doplnok rozhodne automaticky. Pri nerozpoznanom formáte ponúkne klávesovú voľbu.
- Upravený presun fokusu pri veľkých náhľadoch, pri ktorých NVDA mohol zamrznúť.
- Pri prvom zablokovanom stiahnutí v aktívnom náhľade sa zobrazí vysvetlenie.

## 0.7.1 — 2026-11-07

- Skok na zdroj teraz používa označenie zo stránky bez kopírovania do schránky. Pri NVDA v režime prehliadania použite Formátovaný výber.
- HTML aj Markdown náhľady majú samostatné okná Windowsu. Tým sa opravil návrat cez Alt+Tab po skoku na zdroj a zachovanie veľkosti okna pri prepnutí.

## 0.7 — 2026-11-07

- Prvý variant skoku cez Alt+Shift+S hľadal riadok podľa natívnej pozície textového kurzora a jednoznačnej zhody. Pri chýbajúcej pozícii alebo zhode zobrazil hlášku.

## 0.6.1 — 2026-11-07

- HTML aj Markdown náhľady sa otvárajú maximalizované. Obnovenie maximalizuje existujúce okno.
- Dokončenie načítania sa páruje s konkrétnou navigáciou. Zablokovaná iná navigácia už sama neukončí čakanie na načítanie dokumentu.
- Doplnený samostatný 30-sekundový limit na inicializáciu a načítanie. Pri hlásenom zlyhaní WebView2 možno skúsiť náhľad obnoviť z editora.

## 0.6 — 2026-10-18

- Doplnok pri používaní nepotrebuje samostatný Visual C++ Redistributable ani WebView2Loader.dll.
- Ak chýba WebView2 Runtime, doplnok zobrazí postup a odkaz na Microsoft. Runtime sa inštaluje osobitne.

## 0.5 — 2026-10-18

- Pridaný Markdown náhľad a obnova cez Alt+Shift+M. Samostatný náhľad podporuje UTF-8 súbory `.md` a `.markdown` bez ohľadu na veľkosť písmen v prípone.
- Markdown sa spracúva lokálne cez MD4C. Zobrazuje tabuľky, zoznamy úloh, prečiarknutie, poznámky pod čiarou, kód a vložené HTML.

## 0.4.1 — 2026-10-18

- Opravená chyba 14 pri prvom načítaní. WebView2 teraz dostane povolenie načítať očakávanú adresu náhľadu.

## 0.4 — 2026-10-18

- Pridaný samostatný náhľad HTML a príkaz Alt+Shift+H pre aktívnu kartu vrátane neuložených úprav.
- Každý dokument má vlastné okno. Po zatvorení sa doplnok pokúsi vrátiť fokus, kurzor a výber do editora.
- Náhľad poskytuje HTML z pamäte, podporuje vzdialené HTTPS zdroje a otvára aktivované webové odkazy v predvolenom prehliadači.
- Zablokované rámce, sťahovanie, citlivé oprávnenia a prístup dokumentu k natívnym funkciám editora.
