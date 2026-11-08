# npp-html-markdown-preview 0.8.1

**Skúšobná zostava pre Notepad++.** Doplnok npp-html-markdown-preview jednoduchým a prístupným spôsobom vykreslí náhľad súborov písaných primárne v značkovacích jazykoch HTML a Markdown. Náhľad sa otvorí z aktívnej karty v samostatnom maximalizovanom okne a zahŕňa aj neuložené zmeny. Funkcie, ovládanie a prístupnosť navrhla autorka projektu.

### Požiadavky

- Windows 11 a Notepad++ x64.
- Microsoft Edge WebView2 Evergreen Runtime, ktorý Windows 11 zvyčajne už obsahuje. Ak chýba, použite **Evergreen Bootstrapper** s internetovým pripojením alebo **Evergreen Standalone Installer x64** offline. Potom znova spustite Notepad++. [Stiahnutie WebView2 od Microsoftu](https://developer.microsoft.com/en-us/microsoft-edge/webview2/)
- Visual Studio ani samostatný Visual C++ Redistributable na používanie doplnku netreba.

### Inštalácia alebo aktualizácia

1. Zatvorte všetky okná Notepad++. Ak prechádzate z verzie Accessible Preview, presuňte starý podpriečinok `plugins\AccessiblePreview` mimo `plugins`. Ak ste dostali ZIP, rozbaľte ho.
2. V Prieskumníkovi označte **npp-html-markdown-preview.dll** z vydania a stlačte **Ctrl+C**.
3. Stlačte **Alt+D**, zadajte cestu k priečinku `plugins` a stlačte **Enter**. Pri bežnej inštalácii je to `C:\Program Files\Notepad++\plugins`. Pri prenosnej verzii je `plugins` vedľa `notepad++.exe`.
4. Otvorte podpriečinok **npp-html-markdown-preview**. Ak chýba, vytvorte ho cez **Ctrl+Shift+N**, zadajte názov, potvrďte cez **Enter** a otvorte ho. Vložte DLL cez **Ctrl+V**. Pri aktualizácii doplnku s tým istým názvom potvrďte nahradenie pôvodnej DLL. Windows môže pri kopírovaní do Program Files vyžiadať oprávnenie správcu.
5. Spustite Notepad++ bežným spôsobom. V ponuke **Pluginy** nájdete **npp-html-markdown-preview**.

Výsledná cesta pri bežnej inštalácii je `C:\Program Files\Notepad++\plugins\npp-html-markdown-preview\npp-html-markdown-preview.dll`. Zachovajte názov DLL aj podpriečinka. Kontrolný súčet je v súbore `SHA256SUMS.txt` pri vydaní.

### Ovládanie

- **Alt+Shift+P** otvorí náhľad. Rovnaká skratka v editore obnoví už otvorený náhľad aktívnej karty. Príkaz **Náhľad / obnoviť** je aj v ponuke doplnku. Skratku možno zmeniť v správcovi skratiek Notepad++.
- Prípony `.html` a `.htm` zvolia HTML. Prípony `.md` a `.markdown` zvolia Markdown. Pri nepomenovanej karte alebo neznámej prípone sa použije HTML, ak ho rozpoznal editor. Inak vyberte formát cez **Tab** a **Enter** alebo voľbu zrušte cez **Escape**. Doplnok si výber zapamätá do zatvorenia karty.
- V náhľade použite šípky alebo rýchlu navigáciu čítača obrazovky. **Tab** a **Shift+Tab** presúvajú fokus, **Enter** aktivuje odkazy a **medzerník** tlačidlá.
- **Alt+Tab** prepína medzi editorom a náhľadmi. **Alt+F4** zavrie aktívny náhľad.
- Ponuka a hlášky doplnku sú v slovenčine.

#### Skok na zdroj s NVDA

1. V režime prehliadania zapnite **Formátovaný výber** cez **NVDA+Shift+F10**, ak ešte nie je zapnutý.
2. Cez **Shift+šípky** označte niekoľko písmen z jedinečného nadpisu alebo odseku.
3. Stlačte **Alt+Shift+S**. Pri jednoznačnej zhode prejde kurzor v editore na príslušný riadok. Cez **Alt+Tab** sa vrátite do náhľadu.

Po úprave zdroja pred skokom obnovte náhľad. Opakovaný alebo dynamicky vytvorený text nemusí mať jednoznačnú zhodu. Skok s NVDA bol overený vo verzii 0.7.1. Skok s JAWS zatiaľ zostáva neoverený.

### Obmedzenia a bezpečnosť

- Náhľad môže automaticky načítať obrázky, štýly, písma a skripty cez HTTPS. Skript môže odoslať údaje z článku na internet. Používajte dôveryhodné dokumenty a zdroje.
- Miestne obrázky, štýly a odkazy na miestne HTML či Markdown súbory táto verzia nepodporuje. Webový odkaz otvorí predvolený prehliadač. Limit vstupu je 32 MiB.
- Sťahovanie z náhľadu je zablokované. Prvý zablokovaný pokus v aktívnom náhľade zobrazí vysvetlenie, ktoré zavriete cez **Enter** alebo **Escape**. Na uloženie súboru otvorte stránku v bežnom prehliadači.

### Odinštalovanie

Pri zatvorenom Notepad++ odstráňte priečinok `plugins\npp-html-markdown-preview`. WebView2 ponechajte k dispozícii pre ostatné aplikácie.
