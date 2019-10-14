/************************************************
 * HTTP nástěnka v C/C++
 * Autor: Martin Vlach [xvlach18@stud.fit.vutbr.cz]
 * Předmět: ISA - Síťové aplikace a správa sítí
 * Soubor: isaserver.cpp
 ************************************************/


// TODO: oddelat debugFlag

#include <thread>
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
    regex intRegex ("^([0-9]+)$");
    smatch matches;

    if(regex_search(portNum,matches,intRegex)) // Kontrola zda se vůbec jedná o číslo
    {
        string::size_type st;
        PORT = stoi(portNum,&st);
        if(!(PORT >= 0 && PORT <= 65535))
            Error(PORT_BAD_RANGE);
        return;
    }
    Error(PORT_BAD_RANGE);
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
        IsLegitPortNumber(argumentsArray[2]); // Ověření zda se jedná o validní číslo portu, případně se toto číslo nastaví
        return;
    }
    Error(UNKNOWN_ARGUMENT); // Neznámý uživatelský argument
}

struct addrinfo *LoadAddressInfo()
{
    int returnCodeGAI; // Návratový kód funce getaddrinfo()
    struct  addrinfo *addressInfo; // Získaná struktura dat o adrese
    struct addrinfo hints; // hints nastavení (okoukáno z méno projektu IPK2)

    memset(&hints,0, sizeof(hints)); // Vynulování hints
    hints.ai_family = AF_INET; // IPv4
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;


    // Potřebuju PORT (int) dostat jako char*
    stringstream portStringStream;
    portStringStream << PORT;
    string portString = portStringStream.str();
    char* port = (char*) portString.c_str();

    returnCodeGAI = getaddrinfo(nullptr,port,&hints,&addressInfo);
    if(returnCodeGAI != OK) // Nepodařilo se získat info konkrétní popisy chyb zde
    {
        perror("LINE 113 - CheckAddressInfo() - : ");
        Error(NULL_ADDRESS_INFO);
    }
    if(DEBUG)
        cout << "OK - Have address info" << endl;
    return addressInfo;
}

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

        // Podařilo se mi otevřít socket a nabindovat se na
        failed = false;
        if(DEBUG)
            cout << "OK - Socket created and binded" << endl;
        break;
    }

    free(addressInfo);

    if (failed) // Nepodařilo se mi žádný socket otevřít a nabindovat se na něj
    {
        if(DEBUG)
            perror("LINE 160 - OpenSocket() - : ");
        return FAIL;
    }
    return socketFD; // Vrátím File Descriptor socketu
}

int ServerStart()
{
    int socketFD;
    struct addrinfo *addressInfo; // Struktura s informacemi o adrese
    addressInfo = LoadAddressInfo(); // Pokusím se získat data o adrese, předání z pole argumentů, protože port mám jako int a je potřeba jako *char
    CheckAddressInfo(addressInfo); // Kontrola že se mě data podařilo získat
    if((socketFD = OpenSocket(addressInfo)) == FAIL) // Nedostal jsem validní číslo file descriptoru socketu
        Error(SOCKET_ERROR);
    if (listen(socketFD,QUEUE_LEN) != OK) // Nepodařiloe se mi vytvořit frontu požadavků
        Error(QUEUE_ERROR);

    return socketFD;
}

void RequestResolver(int ClientSocket) {
    char buffer[BUFFER_SIZE]; // Vytvoření bufferu pro požadavek na server
    stringstream responseBuffer;
    string response;
    recv(ClientSocket, buffer, BUFFER_SIZE, 0); // Přijmutí požadavku na server do bufferu
    string request(buffer);

    regex boardsRegex("^GET /boards HTTP/1.1\r\n");
    smatch matchesBoards;

    regex addBoardRegex("^POST /boards/([a-zA-Z0-9]+) HTTP/1.1\r\n");
    smatch matchesAddBoard;

    regex deleteBoardRegex("^DELETE /boards/([a-zA-Z0-9]+) HTTP/1.1\r\n");
    smatch matchesDeleteBoard;

    regex boardListRegex ("^GET /board/([a-zA-Z0-9]+) HTTP/1.1\r\n");
    smatch matchesBoardList;

    regex itemAddRegex ("^POST /board/([a-zA-Z0-9]+) HTTP/1.1\r\n");
    smatch matchesItemAdd;

    regex itemDeleteRegex("^DELETE /board/([a-zA-Z0-9]+)/([0-9]+) HTTP/1.1\r\n");
    smatch matchesItemDelete;

    regex itemUpdateRegex("^PUT /board/([a-zA-Z0-9]+)/([0-9]+) HTTP/1.1\r\n");
    smatch matchesItemUpdate;

    regex contentLenRegex ("Content-Length: (\\d+)");
    smatch matchesContentLen;

    regex newLineRegex("\\\\n"); // Potřebuju korektně tisknout nové řádky zadané uživatelem


    if (regex_search(request, matchesBoards, boardsRegex)) // Výpis všech nástěnek
    {
        stringstream tmpBuffer;
        string boards;
        responseBuffer << "HTTP/1.1 200 OK\r\n";

        for (int index = 0; index < BOARDS.size(); index++)
            tmpBuffer << BOARDS[index][0] << "\n";

        boards = tmpBuffer.str();
        responseBuffer << "Content-Type: text/plain\r\n";
        responseBuffer << "Content-Length: " << boards.length() << "\r\n\r\n";
        responseBuffer << boards;
    }

    else if (regex_search(request,matchesAddBoard,addBoardRegex)) // Vložení nové nástěnky
    {
        string name = matchesAddBoard[1];
        bool alreadyIn = false;

        for(int index  = 0; index < BOARDS.size(); index++)
        {
            if(name == BOARDS[index][0])
                alreadyIn = true;
        }

        if(alreadyIn)
            responseBuffer << "HTTP/1.1 409 CONFLICT\r\n\r\n";
        else
        {
            BOARDS.push_back(vector<string>{name});
            responseBuffer << "HTTP/1.1 201 OK\r\n\r\n";
        }
    }

    else if (regex_search(request,matchesDeleteBoard,deleteBoardRegex)) // Smazání nástěnky
    {
        bool isIn = false;
        string name = matchesDeleteBoard[1];
        int index;
        for(index = 0; index < BOARDS.size(); index++)
        {
            if(name == BOARDS[index][0])
            {
                isIn = true;
                break;
            }
        }
        if (isIn)
        {
            BOARDS.erase(BOARDS.begin() + index); // Odstraním hledanou nástěnku ze seznamu
            responseBuffer << "HTTP/1.1 200 OK\r\n\r\n";
        }
        else
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
    }

    else if (regex_search(request,matchesBoardList,boardListRegex))
    {
        string name = matchesBoardList[1];
        bool isIn = false;
        int index;
        for(index = 0; index < BOARDS.size(); index++)
        {
            if(name == BOARDS[index][0])
            {
                isIn = true;
                break;
            }
        }
        if(!isIn)
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
        else
        {
            stringstream tmpBuffer;
            string boardContent;

            responseBuffer << "HTTP/1.1 200 OK\r\n";

            tmpBuffer << "[" << name << "]" << "\n";
            for (int indexIN = 1; indexIN < BOARDS[index].size(); indexIN++)
            {
                tmpBuffer <<  indexIN << ". " << BOARDS[index][indexIN] << "\n";

            }
            boardContent = tmpBuffer.str();
            boardContent = tmpBuffer.str();
            responseBuffer << "Content-Type: text/plain\r\n";
            responseBuffer << "Content-Length: " << boardContent.length() << "\r\n\r\n";
            responseBuffer << boardContent;
        }
    }

    else if (regex_search(request,matchesItemAdd,itemAddRegex))
    {
        string name = matchesItemAdd[1];
        bool isIn = false;
        int index;
        for(index = 0; index < BOARDS.size(); index++)
        {
            if(name == BOARDS[index][0])
            {
                isIn = true;
                break;
            }
        }
        if(!isIn)
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
        else
        {
            if(regex_search(request,matchesContentLen,contentLenRegex))
            {
                int contentLen;
                istringstream contentLenStream (matchesContentLen[1]);
                contentLenStream >> contentLen;

                if(contentLen > 0)
                {
                    string content;
                    content = request.substr(request.length() - contentLen);
                    content = regex_replace(content,newLineRegex,"\n");
                    BOARDS[index].push_back(content);
                    responseBuffer << "HTTP/1.1 201 OK\r\n\r\n";
                }
                else
                    responseBuffer << "HTTP/1.1 400 BAD REQUEST\r\n\r\n";
            }
        }
    }
    else if (regex_search(request,matchesItemDelete,itemDeleteRegex))
    {
        string name (matchesItemDelete[1]);
        int contributionIndex;
        istringstream contributionIndexStream (matchesItemDelete[2]);
        contributionIndexStream >> contributionIndex;

        bool isIn = false;
        int index;

        for (index = 0; index < BOARDS.size(); index++)
        {
            if(BOARDS[index][0] == name)
            {
                isIn = true;
                break;
            }
        }
        if(!isIn)
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
        else
        {
            if((contributionIndex < 1) || (contributionIndex >= (BOARDS[index].size())))
                responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
            else
            {
                BOARDS[index].erase(BOARDS[index].begin() + contributionIndex);
                responseBuffer << "HTTP/1.1 200 OK\r\n\r\n";
            }
        }
    }
    else if (regex_search(request,matchesItemUpdate,itemUpdateRegex)) // Aktualizace obsahu příspěvku na nástěnce
    {
        string name (matchesItemUpdate[1]);
        int contributionIndex;
        istringstream contributionIndexStream (matchesItemUpdate[2]);
        contributionIndexStream >> contributionIndex;

        bool isIn = false;
        int index;
        for (index = 0; index < BOARDS.size(); index++)
        {
            if(BOARDS[index][0] == name)
            {
                isIn = true;
                break;
            }
        }
        if(!isIn)
            responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
        else
        {
            if((contributionIndex < 1) || (contributionIndex >= (BOARDS[index].size())))
                responseBuffer << "HTTP/1.1 404 NOT FOUND\r\n\r\n";
            else
            {
                if(regex_search(request,matchesContentLen,contentLenRegex))
                {
                    int contentLen;
                    istringstream contentLenStream (matchesContentLen[1]);
                    contentLenStream >> contentLen;

                    if(contentLen > 0)
                    {
                        string content;
                        content = request.substr(request.length() - contentLen);
                        content = regex_replace(content,newLineRegex,"\n");
                        BOARDS[index][contributionIndex] = content;
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

    response = responseBuffer.str();
    send(ClientSocket,response.c_str(),response.length(),0);
    close(ClientSocket);
}

int ServerRun(int serverFD)
{
    // TODO popis
    int newSocket;
    struct sockaddr_storage client;
    socklen_t size = sizeof(client);

    vector<thread> threadsVector; // Paraelismus

    while(true)
    {
       if((newSocket = accept(serverFD, (struct sockaddr *)&client, &size)) == FAIL)
           Error(ACCEPT_ERROR);
        threadsVector.push_back(thread(RequestResolver,newSocket));
    }
    for (std::thread & THREAD : threadsVector)
    {
        if (THREAD.joinable()) // Počkání na synovské vlákna
            THREAD.join();
    }
    close(serverFD);
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
