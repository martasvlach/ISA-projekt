/************************************************
 * HTTP nástěnka v C/C++
 * Autor: Martin Vlach [xvlach18@stud.fit.vutbr.cz]
 * Předmět: ISA - Síťové aplikace a správa sítí
 * Soubor: isaserver.cpp
 ************************************************/

#include "isaserver.h"

using namespace std;

void PrintHelp()
{
    cout << "=============================================" << endl;
    cout << "Spuštění -- ./isaserver -p <port>" << endl;
    cout << " - <port> je číslo portu na kterém má server očekávat spojení" << endl;
    cout << "          Toto číslo musí být validní číslo portu (z intervalu <0;65535>)" << endl;
    cout << endl;
    cout << "  == Příklad spuštění ==" << endl;
    cout << "      ./isaserver -p 80" << endl;
    cout << endl;
    cout << "  == Autor ==" << endl;
    cout << "      Martin Vlach [xvlach18@stud.fit.vutbr.cz]" << endl;
    cout << "=============================================" << endl;
    exit(OK);
    // Návratový kód po vypsání nápovědy je 0
}

void Error(int errorCode)
{
    switch (errorCode)
    {
        case TOO_FEW_ARGUMENTS:
            cerr << "Nebyl zadán žádný uživatelský argument .. \n použití ./isaserver -p <port>" << endl;
            exit(TOO_FEW_ARGUMENTS);
        case TOO_MANY_ARGUMENTS:
            cerr << "Bylo zadáno příliš mnoho argumentů pro tuto jejich kombinaci" << endl;
            exit(TOO_MANY_ARGUMENTS);
        case PORT_NUM_UNSPECIFIED:
            cerr << "Nebylo zadáno číslo portu na kterém má server očekávat spojení" << endl;
            exit(PORT_NUM_UNSPECIFIED);
        case UNKNOWN_ARGUMENT:
            cerr << "Zadaný argument nebyl rozpoznán" << endl;
            exit(UNKNOWN_ARGUMENT);
        case PORT_BAD_RANGE:
            cerr << "Zadaný port není číslem v rozsahu <0;65535>" << endl;
            exit(PORT_BAD_RANGE);
        case NULL_ADDRESS_INFO:
            cerr << "Nepodařilo se zjistit potřebné informace o adrese, nutné pro otevření socketu" << endl;
            exit(NULL_ADDRESS_INFO);
        case SOCKET_ERROR:
            cerr << "Nepodařilo se otevřít nebo nabindovat socket na zadanén portu" << endl;
            exit(SOCKET_ERROR);
        case QUEUE_ERROR:
            cerr << "Nepodařilo se vytvořit frontu požadavků" << endl;
            exit(QUEUE_ERROR);
        case ACCEPT_ERROR:
            cerr << "Nepodařilo se spustit naslouchání požadavků na serveru" << endl;
            exit(ACCEPT_ERROR);
        default:
            break;
    }
}

void IsLegitPortNumber(char *pn)
{
    string portNum (pn);
    regex intRegex ("^([0-9]+)$"); // Kontrola zda se vůbec jedná o číslo
    smatch matches;

    if(regex_search(portNum,matches,intRegex)) 
    {
        string::size_type st;
        PORT = stoi(portNum,&st);
        if(!(PORT >= 0 && PORT <= 65535)) // Ověření, že se jedná o port ve validním rozsahu (<0;65535>)
            Error(PORT_BAD_RANGE);
        return; // Kontrola validnosti čísla portu proběhla v pořádku
    }
    Error(PORT_BAD_RANGE); // Nejednalo se o číslo
}

void ParseArguments(int argumentsCount, char **argumentsArray)
{
    if(argumentsCount < 2) // Nebyl zadán žádný uživatelský argument
        Error(TOO_FEW_ARGUMENTS);
    if(strcmp(argumentsArray[1],"-h") == 0)
    {
        if(argumentsCount == 2)
            PrintHelp();
        else
            Error(TOO_MANY_ARGUMENTS);
    }
    if(strcmp(argumentsArray[1],"-p") == 0)
    {
        if(argumentsCount < 3) // Číslo portu nebylo uživatelem zadáno
            Error(PORT_NUM_UNSPECIFIED);
        IsLegitPortNumber(argumentsArray[2]); // Ověření zda se jedná o validní číslo portu, případně se toto číslo nastaví do PORT
        return;
    }
    Error(UNKNOWN_ARGUMENT); // Neznámý uživatelský argument
}

struct addrinfo *LoadAddressInfo()
{
    int returnCodeGAI; // Návratový kód funce getaddrinfo()
    struct  addrinfo *addressInfo; // Získaná struktura dat o adrese
    struct addrinfo hints; // hints nastavení (okoukáno z méno projektu IPK2)
    
    // Vynulování hints + nastavení hints
    memset(&hints,0, sizeof(hints)); 
    hints.ai_family = AF_INET; 
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;


    // Potřebuju PORT (int) dostat jako char* -: volání funkce getaddrinfo()
    stringstream portStringStream;
    portStringStream << PORT;
    string portString = portStringStream.str();
    char* port = (char*) portString.c_str();

    returnCodeGAI = getaddrinfo(nullptr,port,&hints,&addressInfo);
    if(returnCodeGAI != OK) // Nepodařilo se získat addrinfo
        Error(NULL_ADDRESS_INFO);
    if(DEBUG)
        cout << "OK - Have address info" << endl;
    return addressInfo; // Podařilo se mě získat potřebné informace o adrese -: předávám
}

// Otestování, zda nemám pouze null hodnotu
void CheckAddressInfo(struct addrinfo *addressInfo)
{
    if(addressInfo == nullptr)
    {
        if(DEBUG)
            perror("LINE 124 - CheckAddressInfo() - : ");
        Error(NULL_ADDRESS_INFO);
    }
    if (DEBUG)
        cout << "OK - Adress info is not null" << endl;
}

int OpenSocket(struct addrinfo *addressInfo)
{
    if(addressInfo == nullptr) // Ověření že mám potřené data o adrese
        return FAIL;

    int socketFD; // File Descriptor socketu
    bool failed = true; // Příznak toho, zda se mi úspěšně podařilo otevřít a nabindovat socket

    // Projdu všechny výsledky a pokusím se otevřit a nabindovat na první na kterém se mi to podaří
    for( ; addressInfo != nullptr; addressInfo = addressInfo->ai_next)
    {
        // Pokusím se otevřít socket
        if((socketFD = socket(addressInfo->ai_family, addressInfo->ai_socktype,0)) == FAIL) // Nepodařilo se mi otevřít socket zkusím další pokud je
            continue;
        if((bind(socketFD,addressInfo->ai_addr,addressInfo->ai_addrlen)) == FAIL) // Nepodařil se mi bind, zkusím jiný vytvořit jiný socket a následně se nabindovat na něm, pokud je
        {
            close(socketFD); // Uzavřu socket
            continue;
        }

        failed = false; // Podařilo se mi otevřít socket a provést následný binding
        if(DEBUG)
            cout << "OK - Socket created and binded" << endl;
        break;
    }

    free(addressInfo); // Uvolnění paměti, kde jsem si držel addrinfo

    if (failed) // Nepodařilo se mi žádný socket otevřít a nabindovat se na něj
    {
        if(DEBUG)
            perror("LINE 160 - OpenSocket() - : ");
        return FAIL;
    }
    return socketFD; // Vrátím získaný File Descriptor socketu
}

int ServerStart()
{
    int socketFD; // File Descriptor socketu
    struct addrinfo *addressInfo; // Struktura s informacemi o adrese
    addressInfo = LoadAddressInfo(); // Pokusím se získat data o adrese
    CheckAddressInfo(addressInfo); // Kontrola že se mě data podařilo získat
    if((socketFD = OpenSocket(addressInfo)) == FAIL) // Nedostal jsem validní číslo file descriptoru socketu
        Error(SOCKET_ERROR);
    if (listen(socketFD,QUEUE_LEN) != OK) // Nepodařilo se mi vytvořit frontu požadavků
        Error(QUEUE_ERROR);

    return socketFD; // Vracím FD socketu na kterém poběží server
}

void RequestResolver(int ClientSocket) {
    char buffer[BUFFER_SIZE]; // Vytvoření bufferu pro požadavek na server
    stringstream responseBuffer;
    string response;
    recv(ClientSocket, buffer, BUFFER_SIZE, 0); // Načtení požadavku klienta do bufferu
    string request(buffer); // Převedu si do stringu, pro možnost práce s regexi .. 

    //Výpis všech nástěnek 
    regex boardsRegex("^GET /boards HTTP/1.1\r\n");
    smatch matchesBoards;

    // Přídání nástěnky 
    regex addBoardRegex("^POST /boards/([a-zA-Z0-9]+) HTTP/1.1\r\n");
    smatch matchesAddBoard;

    // Odstranění nástěnky
    regex deleteBoardRegex("^DELETE /boards/([a-zA-Z0-9]+) HTTP/1.1\r\n");
    smatch matchesDeleteBoard;

    // Výpis obsahu nástěnky
    regex boardListRegex ("^GET /board/([a-zA-Z0-9]+) HTTP/1.1\r\n");
    smatch matchesBoardList;

    // Přídání příspěvku na nástěnku
    regex itemAddRegex ("^POST /board/([a-zA-Z0-9]+) HTTP/1.1\r\n");
    smatch matchesItemAdd;

    // Odstranění příspěvku z nástěnky
    regex itemDeleteRegex("^DELETE /board/([a-zA-Z0-9]+)/([0-9]+) HTTP/1.1\r\n");
    smatch matchesItemDelete;

    // Aktualizace příspěvku na nástěnce
    regex itemUpdateRegex("^PUT /board/([a-zA-Z0-9]+)/([0-9]+) HTTP/1.1\r\n");
    smatch matchesItemUpdate;

    // Zjistění délky <content> ve zprávě od klienta
    regex contentLenRegex ("Content-Length: (\\d+)");
    smatch matchesContentLen;

    // Potřebuju korektně tisknout nové řádky zadané uživatelem (předpokládané zadávání z shellu)
    regex newLineRegex("\\\\n"); 

    // Výpis všech nástěnek
    if (regex_search(request, matchesBoards, boardsRegex)) 
    {
        stringstream tmpBuffer;
        string boards;
        responseBuffer << "HTTP/1.1 200 OK\r\n";

        // Projdu si všechny seznam všech nástěnek a uložím si jejich názvy, přičemž jsou odděleny pomocí '\n'
        for (unsigned long int index = 0; index < BOARDS.size(); index++)
            tmpBuffer << BOARDS[index][0] << "\n";

        boards = tmpBuffer.str();
        responseBuffer << "Content-Type: text/plain\r\n";
        // Zjištění délky užitného obsahu zprávy, a uvedení tohoto faktu do HTTP hlavičky
        responseBuffer << "Content-Length: " << boards.length() << "\r\n\r\n";
        responseBuffer << boards;
    }

    // Vložení nové nástěnky
    else if (regex_search(request,matchesAddBoard,addBoardRegex)) 
    {
        string name = matchesAddBoard[1]; // Jméno nástěnky kterou chci vytvořit
        bool alreadyIn = false; // Příznak toho, že nástěnka se stejným jménem již existuje
        for(unsigned long int index  = 0; index < BOARDS.size(); index++)
        {
            if(name == BOARDS[index][0])
                alreadyIn = true;
        }

        if(alreadyIn) // V "databázi" je již nástěnka se stejným jménem
            responseBuffer << "HTTP/1.1 409 CONFLICT\r\n\r\n";
        else // Jméno nástěnky je dostupné, vložím tedy práznou nástěnku s požadovaným jménem
        {
            BOARDS.push_back(vector<string>{name});
            responseBuffer << "HTTP/1.1 201 OK\r\n\r\n";
        }
    }

    // Smazání nástěnky
    else if (regex_search(request,matchesDeleteBoard,deleteBoardRegex)) 
    {
        string name = matchesDeleteBoard[1]; // Jméno nástěnky
        bool isIn = false; // Příznak toho, zda existuje nástěnky s požadovaným jménem
        unsigned long int index; // Na jakém indexu s seznamu nástěneḱ se požadovaná nástěnka nachází
        for(index = 0; index < BOARDS.size(); index++)
        {
            if(name == BOARDS[index][0])
            {
                isIn = true;
                break;
            }
        }
        if (isIn) // Nástěnka s požadovaným jménem existuje
        {
            BOARDS.erase(BOARDS.begin() + index); // Odstraním hledanou nástěnku ze seznamu
            responseBuffer << "HTTP/1.1 200 OK\r\n\r\n";
        }
        else // Nástěnka s požadovaným jménem NEexistuje
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
    }

    // Výpis všech příspěvků na zadané nástěnce
    else if (regex_search(request,matchesBoardList,boardListRegex))
    {
        string name = matchesBoardList[1]; // Jméno nástěnky
        bool isIn = false; // Příznak toho, zda existuje nástěnky s požadovaným jménem
        unsigned  long int index; // Na jakém indexu s seznamu nástěneḱ se požadovaná nástěnka nachází
        for(index = 0; index < BOARDS.size(); index++)
        {
            if(name == BOARDS[index][0])
            {
                isIn = true;
                break;
            }
        }
        if(!isIn) // Nástěnka s požadovaným jménem v seznamu NEexistuje
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
        else
        {
            stringstream tmpBuffer; // Pomocný buffer
            string boardContent; // Výčet všech příspěvků ná nástěnce

            responseBuffer << "HTTP/1.1 200 OK\r\n";

            tmpBuffer << "[" << name << "]" << "\n"; // Nadpis jméno nástěnky ve formátu [NAME]
            for (unsigned long int indexIN = 1; indexIN < BOARDS[index].size(); indexIN++) // Načtu do bufferu všechny příspěvky v nástěnce + jejich pořadí
            {
                tmpBuffer <<  indexIN << ". " << BOARDS[index][indexIN] << "\n";

            }
            boardContent = tmpBuffer.str();
            boardContent = tmpBuffer.str();
            responseBuffer << "Content-Type: text/plain\r\n";
            responseBuffer << "Content-Length: " << boardContent.length() << "\r\n\r\n"; // Zjistění užitečné délky obsahu a následný zápis této hodnoty do HTTP hlavičky
            responseBuffer << boardContent;
        }
    }

    // Přídání příspěvku do nástěnky
    else if (regex_search(request,matchesItemAdd,itemAddRegex)) 
    {
        string name = matchesItemAdd[1]; // Jméno nástěnky
        bool isIn = false; // Příznak toho, zda existuje nástěnky s požadovaným jménem
        unsigned long int index; // Na jakém indexu s seznamu nástěneḱ se požadovaná nástěnka nachází
        for(index = 0; index < BOARDS.size(); index++)
        {
            if(name == BOARDS[index][0])
            {
                isIn = true;
                break;
            }
        }
        if(!isIn) // Nástěnka s požadovaným jménem v seznamu NEexistuje
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
        else // Nástěnky s požadovým jménem existuje
        {
            if(regex_search(request,matchesContentLen,contentLenRegex)) // Zjisžění délky užitečného obsahu ve správě
            {
                int contentLen;
                istringstream contentLenStream (matchesContentLen[1]);
                contentLenStream >> contentLen;

                if(contentLen > 0) // Samotné přídání příspěvku do nástěnky
                {
                    string content;
                    content = request.substr(request.length() - contentLen);
                    content = regex_replace(content,newLineRegex,"\n"); // Nahrazení znaků nových řádků u víceřádkových příspěvků
                    BOARDS[index].push_back(content);
                    responseBuffer << "HTTP/1.1 201 OK\r\n\r\n";
                }
                else // Nulová velikost obsahu
                    responseBuffer << "HTTP/1.1 400 BAD REQUEST\r\n\r\n";
            }
        }
    }
    // Odstranění příspěvku z nástěnky
    else if (regex_search(request,matchesItemDelete,itemDeleteRegex)) 
    {
        string name (matchesItemDelete[1]); // Jméno nástěnky
        unsigned long int contributionIndex; // Index příspěvku v nástěnce
        istringstream contributionIndexStream (matchesItemDelete[2]);
        contributionIndexStream >> contributionIndex;

        bool isIn = false; // Příznak jestli nástěnka se zadaným jménem existuje
        unsigned long int index; // Index na kterém se nachází nástěnka v seznamu

        for (index = 0; index < BOARDS.size(); index++)
        {
            if(BOARDS[index][0] == name)
            {
                isIn = true;
                break;
            }
        }
        if(!isIn) // Nástěnka se zadaným jménem neexstuje
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
        else
        {
            // Ověření že příspěvěk který chci mazat na nástěnce opravdu existuje
            if((contributionIndex < 1) || (contributionIndex >= (BOARDS[index].size()))) // Neexistuje
                responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
            else
            {
                BOARDS[index].erase(BOARDS[index].begin() + contributionIndex); // Smazání příspěvku s pořadím <id>
                responseBuffer << "HTTP/1.1 200 OK\r\n\r\n";
            }
        }
    }

    // Aktualizace obsahu příspěvku na nástěnce
    else if (regex_search(request,matchesItemUpdate,itemUpdateRegex)) 
    {
        string name (matchesItemUpdate[1]); // Jméno nástěnku
        unsigned long int contributionIndex; // Pozice příspěvku v nástěnce
        istringstream contributionIndexStream (matchesItemUpdate[2]);
        contributionIndexStream >> contributionIndex;

        bool isIn = false; // Příznak zda existuje nástěnka s požadovaným name
        unsigned long int index; // Pozice nástěnky ve seznamu
        for (index = 0; index < BOARDS.size(); index++)
        {
            if(BOARDS[index][0] == name)
            {
                isIn = true;
                break;
            }
        }
        if(!isIn) // Nástěnka s požadovaným name neexistuje
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
        else
        {
            // Ověření že příspěvěk který chci updatovat na nástěnce opravdu existuje
            if((contributionIndex < 1) || (contributionIndex >= (BOARDS[index].size()))) // neexistuje
                responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
            else
            {
                if(regex_search(request,matchesContentLen,contentLenRegex)) // Existuje
                {
                    int contentLen;
                    istringstream contentLenStream (matchesContentLen[1]);
                    contentLenStream >> contentLen;

                    if(contentLen > 0)
                    {
                        string content;
                        content = request.substr(request.length() - contentLen); 
                        content = regex_replace(content,newLineRegex,"\n");
                        BOARDS[index][contributionIndex] = content; // Update příspěvku
                        responseBuffer << "HTTP/1.1 200 OK\r\n\r\n";
                    }
                    else
                        responseBuffer << "HTTP/1.1 400 BAD REQUEST\r\n\r\n";
                }
            }
        }
    }

    else // Nedostal jsem v požavku nic známého
    {
        responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
    }

    response = responseBuffer.str(); // Načtu hodnoty z bufferu do odpovědi
    send(ClientSocket,response.c_str(),response.length(),0); // Odeslání odpovědi na požadavek klientovi
    close(ClientSocket); // Zavření socketu, kde se zpracovávala odpověď
}

int ServerRun(int serverFD)
{
    int newSocket; // Nový socket, poběží v novém vlákně, kde se bude zpracovávat požadavek, mateřský bude nadále naslouchat
    struct sockaddr_storage client; // Klient
    socklen_t size = sizeof(client); // Klient

    vector<thread> threadsVector; // Vlákna pro zpracování požadavků klientů

    while(true) // Server běží v infinity loopu, dokud není "sestřelen"
    {
       if((newSocket = accept(serverFD, (struct sockaddr *)&client, &size)) == FAIL)
           Error(ACCEPT_ERROR);
        threadsVector.push_back(thread(RequestResolver,newSocket)); // Přídám nové vlákno, a spustím v něm zpracování požadavku
    }
    for (std::thread & THREAD : threadsVector) // Projdu všechny vlákna a kouknu se zda v některém stále běží spracování požadavku
    {
        if (THREAD.joinable()) // Pokud běží
            THREAD.join(); // Počkám dokud se neukončí
    }
    close(serverFD); // Zavírám hlavní socket serveru (na kterém naslouchal)
    return OK;
}

int main(int argc, char **argv)
{
    ParseArguments(argc,argv); // Zparsování argumentů
    int serverSocket;
    serverSocket = ServerStart(); // Rozběhnutí serveru a navrácení file descriptoru socketu serveru
    ServerRun(serverSocket);
    return OK;
}

/* end isaserver.cpp */
