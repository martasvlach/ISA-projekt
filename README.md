Nástenky

Nástenkou sa rozumie usporiadaný zoznam textových (ASCII) príspevkov. Každý príspevok ma id (číslované od 1) a textový obsah, ktorý môže byť viacriadkový. id nie je permanentné, korešponduje aktuálnej pozícii v zozname. Operácie nad nástenkou by nemali meniť poradie príspevkov. Formát zobrazenia nástenky je:

[názov] 
1. Prvý príspevok. 
2. Druhý príspevok. 
... 
n. N-tý (posledný príspevok). 
Príklad nástenky:

[priklad] 
1. Jednoriadkovy prispevok. 
2. Viacriadkovy prispevok. 
Pokracuje na druhom riadku. 
3. Dalsi jednoriadkovy prispevok. 
Po zmazaní príspevku 2. bude nástenka vyzerať následovne:

[priklad] 
1. Jednoriadkovy prispevok. 
2. Dalsi jednoriadkovy prispevok. 
HTTP API

Aplikácie medzi sebou komunikujú pomocou protokolu HTTP a rozhrania nad ním. Použitá verzia HTTP je 1.1. Stačí použiť minimálny počet hlavičiek potrebných na správnu komunikáciu, aplikácia by však mala byť schopná vysporiadať sa aj s neznámymi hlavičkami (t.j. preskočiť a ignorovať). V prípadoch kedy sa posielajú dáta je použitý Content-Type:text/plain. Riadky obsahu sú oddelené len \n.

API je definované následovne:

GET /boards - Vráti zoznam dostupných nástenok, jedna na riadok.
POST /boards/name - Vytvorí novú prázdnu nástenku s názvom name.
DELETE /boards/name - Zmaže nástenku name a všetok jej obsah.
GET /board/name - Zobrazí obsah nástenky name.
POST /board/name - Vloží nový príspevok do nástenky name. Príspevok je vložený na koniec zoznamu.
PUT /board/name/id - Zmení obsah príspevku číslo id v nástenke name.
DELETE /board/name/id - Zmaže príspevok číslo id z nástenky name.
GET, PUT a DELETE vracajú pri úspechu kód 200, POST 201. Pokiaľ žiadaná nástenka alebo príspevok neexistujú, vracia sa kód 404. Pokiaľ je snaha vytvoriť nástenku s názvom ktorý už existuje, vracia sa kód 409. Pokiaľ POST alebo PUT nad príspevkom majú Content-Length = 0, vracia sa kód 400.

Na všetky iné akcie reaguje server odpoveďou 404.

Rozhranie aplikácií

Skompilovaný klient sa bude volať isaclient, server bude mať názov isaserver.

Oba programy po spustení s parametrom -h vypíšu na stdout informácie o spôsobe spustenia.

Server akceptuje jeden povinný parameter, -p, ktorý určuje port na ktorom bude server očakávať spojenia. (./isaserver -p 5777)

Spôb spustenia klienta vyzerá následovne:

./isaclient -H <host> -p <port> <command>

kde <command> môže byť (za - vždy následuje ekvivalenté API):

boards - GET /boards
board add <name> - POST /boards/<name>
board delete <name> - DELETE /boards/<name>
boards list <name> - GET /board/<name>
item add <name> <content> - POST /board/<name>
item delete <name> <id> - DELETE /board/<name>/<id>
item update <name> <id> <content> - PUT /board/<name>/<id>
Klient vypíše hlavičky odpovede na stderr a obsah odpovede na stdout.

Implementácia

Vaše riešenie by malo spĺňať podmienky popísané v spoločnom zadaní. Projekt bude napísaný v jazyku C/C++ a preložiteľný na serveri merlin. Použitie neštandardných knižníc (libcurl etc.) nie je povolené.
