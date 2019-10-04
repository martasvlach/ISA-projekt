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
            cerr << "Nebyl zadán žádný uživatelský argument" << endl;
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
    struct addrinfo hints; // hints nastavení viz projekt IPK2

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
}

int OpenSocket(struct addrinfo *addressInfo)
{
    if(addressInfo == nullptr) // Ověření že mám potřené data o adrese
        return FAIL;

    int socketFD; //File Descriptor socketu
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
        break;
    }

    free(addressInfo);

    if (failed) // Nepodařilo se mi žádný socket otevřít a nabindovat se na něj
    {
        if(DEBUG)
            perror("LINE 160 - OpenSocket() - : ");
        return FAIL;
    }
    return OK;
}

void ServerStart()
{
    struct addrinfo *addressInfo; // Struktura s informacemi o adrese
    addressInfo = LoadAddressInfo(); // Pokusím se získat data o adrese, předání z pole argumentů, protože port mám jako int a je potřeba jako *char
    CheckAddressInfo(addressInfo); // Kontrola že se mě data podařilo získat
    if((OpenSocket(addressInfo)) != OK)
        Error(SOCKET_ERROR);
}

void ServerRun()
{
    // TODO : TBD
}

int main(int argc, char **argv)
{
    ParseArguments(argc,argv); // Zparsování argumentů
    ServerStart(); // Rozběhnutí serveru

    if(DEBUG)
        DEBUG_USERINPUT();
    cout << "Všechno vypadá OK" << endl;
    return OK;
}

/* end isaserver.cpp */