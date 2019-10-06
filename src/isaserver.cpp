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
        case FORK_ERROR:
            cerr << "Nastala chyba během operace fork()" << endl;
            exit(FORK_ERROR);
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

void RequestResolver(int handler)
{
    char buffer[BUFFER_SIZE]; // Vytvoření bufferu pro požadavek na server
    stringstream responseBuffer;
    string response;
    recv(handler, buffer, BUFFER_SIZE, 0); // Přijmutí požadavku na server do bufferu
    string request (buffer);

    responseBuffer << "HTTP/1.0 200 OK\r\n\r\n";
    response = responseBuffer.str();
    send(handler,response.c_str(),response.length(),0);
}

int ServerRun(int serverFD)
{
    pid_t pid;
    int newSocket;

    struct sockaddr_storage client; // TODO popis
    socklen_t size = sizeof(client);

    while(true)
    {
       if((newSocket = accept(serverFD, (struct sockaddr *)&client, &size)) == FAIL)
           Error(ACCEPT_ERROR);
       if(DEBUG)
           cout << "OK -Accept ()" << endl;

       pid = fork(); // Duplicita procesu pro paralelní zpracování požadavků
       if(DEBUG)
           cout << "OK - fork() happen" << endl;

       if (pid > 0) // Rodičovský proces (poslouchání)
       {
        if(DEBUG)
            cout << "OK - Son will parse request and parrent will listen again" << endl;
        close(newSocket);
       }

       else if (pid == 0) // Synovský proces (zpracování požavku na server)
       {
           close(serverFD);
           RequestResolver(newSocket);
           if(DEBUG)
            cout << "Closing son process - Request resolver" << endl;
           close(newSocket);
           exit(OK);
       }

       else // Fork() selhal
       {
           Error(FORK_ERROR);
       }
    }
    if(DEBUG)
        cout << "Closing origin process" << endl;
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