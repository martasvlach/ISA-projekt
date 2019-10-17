/************************************************
 * HTTP nástěnka v C/C++
 * Autor: Martin Vlach [xvlach18@stud.fit.vutbr.cz]
 * Předmět: ISA - Síťové aplikace a správa sítí
 * Soubor: isaclient.cpp
 ************************************************/

#include "isaclient.h"

using namespace std;

void PrintHelp()
{
   cout << "*******************************************************************************************************" << endl;
   cout << "Spuštění :   ./isaclient -H <host> -p <port> <command>"<< endl;
   cout << "Kde platí následující: " << endl;
   cout << "- <host> .. je hostname serveru"<< endl;
   cout << "- <port> číslo portu v rozmezí <0;65535>" << endl;
   cout << "- <command> může být jedna z následujících variant" << endl;
   cout << "   *  1) boards - Vrátí uživateli seznam všech dostupných nástěnek (1/řádek)" << endl;
   cout << "   *  2) board add <name> - Vytvoří na serveru novou nástěnku s názvem <name>" << endl;
   cout << "   *  3) board delete <name> - Smaže ze serveru nástěnku s názvem <name> a všechen její obsah" << endl;
   cout << "   *  4) boards list <name> - Vrátí obsah z dané nástěnky se jménem <name> (jeden příspěvek / řádek)"<< endl;
   cout << "   *  5) item add <name> <content> - Přidá na nástěnku s názvem <name> příspěvek <content> (Pokud není zadán content, odešle se prázdný)" << endl;
   cout << "   *  6) item delete <name> <id> - Odstraní z nástěnky se jménem <name> příspěvek s číslem (pořadím) <id> (celé kladné číslo > 0)" << endl;
   cout << "   *  7) item update <name> <id> - Aktualizuje na nástěnce se jménem <name> příspěvek s číslem (pořadím) <id> (celé kladné číslo  > 0)" << endl;
   cout << "" << endl;
   cout << "   == Omezení/Spuštění==" << endl;
   cout << "   Nezáleží na pořadí parametrů <host> a <port>" << endl;
   cout << " !! Pokud chcete zadat víceslovný/víceřádkový <content>/<name> je třeba jej uvést do uvozovek" << endl;
   cout << "" << endl;
   cout << "    == Příklad spuštění ==" << endl;
   cout << "   ./isaclient -H localhost -p 80 item add nastenka1 \"Toto je viceslovny prispevek\"" << endl;
   cout <<"   ./isaclient -p 1234 -H localhost item add nastenka2 slovo" << endl;
   cout << "" << endl;
   cout << "    == Autor ==" << endl;
   cout << "      Martin Vlach [xvlach18@stud.fit.vutbr.cz]" << endl;
   exit(OK); 
   // Návratový kód po nápovědě je OK
}

void Error(int errorCode)
{
    switch (errorCode)
    {
        case MISSING_ARGUMENT:
            cerr << "Nebyl zadán žádný argument pro spuštění, spusťte program s přepínačem -h (nápověda)" << endl;
            exit(MISSING_ARGUMENT);
        case TOO_MANY_ARGUMENTS:
            cerr << "Zadáno příliš mnoho argumetů pro tuto kombinaci, spusťte program s přepínačem -h (nápověda)" << endl;
            exit(TOO_MANY_ARGUMENTS);
        case INVALID_PORT_NUMBER:
            cerr << "Zadaný port není číslem v rozsahu <0;65535>" << endl;
            exit(INVALID_PORT_NUMBER);
        case PORT_NOT_SET:
            cerr << "Parametr <port> nebyl nastaven" << endl;
            exit(PORT_NOT_SET);
        case HOST_NOT_SET:
            cerr << "Parametr <host> nebyl nastaven" << endl;
            exit(HOST_NOT_SET);
        case COMMAND_NOT_SET:
            cerr << "Parametr <command> nebyl nastaven" << endl;
            exit(COMMAND_NOT_SET);
        case INVALID_COMMAND:
            cerr << "Nebyl zadán platný <command>, nebo není v platné podobně, spusťte program s přepínačem -h (nápověda)" << endl;
            exit(INVALID_COMMAND);
        case INVALID_ID_NUMBER:
            cerr << "Zadané <id> není platné číslo pro id (celé kladné číslo větší než 0)" << endl;
            exit(INVALID_ID_NUMBER);
        case CONNECT_TO_SERVER_ERROR:
            cerr << "Nepodařilo se připojit k zadanému serveru na zadaném portu, ujistěte se, že je server zapnutý" << endl;
            exit(CONNECT_TO_SERVER_ERROR);
        case NO_SERVER_INFO:
            cerr << "Nepodařilo se získat informace o serveru nutné pro spojení s ním, ověřte vstupní data" << endl;
            exit(NO_SERVER_INFO);
        case SOCKET_CREATE_ERROR:
            cerr << "Nepodařilo se otevřít socket pro komunikaci se serverem" << endl;
            exit(SOCKET_CREATE_ERROR);
        default:
            break;
    }
}

void LoadHost(int argumentsCount, char **argumentsArray)
{
    for(int position = 0; position < (argumentsCount-1);position++) // -1 Kvůli SIGSEG
    {
        if(strcmp(argumentsArray[position],"-H") == 0)
            if ((position+1) < argumentsCount) // Ochrana aby nemohl nastat SIGSEG
            {
                HOST = argumentsArray[position+1];
                break;
            }
    }
}

bool CheckPortNumber(char *pn)
{
    string portNum (pn);
    regex intRegex ("^([0-9]+)$");
    smatch matches;

    if (regex_search(portNum,matches,intRegex)) // Kontrola zda se jedná o číslo
    {
        string::size_type st;
        PORT = stoi(portNum,&st);
        return PORT >= 0 && PORT <= 65535; // Kontrola zda se jedná o validní číslo pro port (<0;65535)
    }
    return false;
}

void VerifyId(char **array,int position)
{
    string idNum;
    idNum = array[position];
    regex idRegex ("^([0-9]+)$");
    smatch matches;

    if (regex_search(idNum,matches,idRegex)) // Kontrola zda se jedná o číslo
    {
        string::size_type st;
        ID = stoi(idNum,&st);
        if (ID < 1)
            Error(INVALID_ID_NUMBER);
        return;
    }
    Error(INVALID_ID_NUMBER);
}

void LoadPort(int argumentsCount, char **argumentsArray)
{
    for (int position = 0; position < (argumentsCount-1);position++) // -1 Kvůli SIGSEG
    {
        if(strcmp(argumentsArray[position],"-p") == 0)
        {
            if((position + 1) < argumentsCount) // Ochrana aby nemohl nastat SIGSEG
            {
                if(!CheckPortNumber(argumentsArray[position+1]))
                    Error(INVALID_PORT_NUMBER);
                break;
            }
        }
    }
}

void SetCheck()
{
    if(PORT == INT_MAGIC) // Hodnota portu nebyla nastavena (PORT má výchozí magickou hodnotu)
        Error(PORT_NOT_SET);
    if(HOST.empty()) // Host nebyl nastaven
        Error(HOST_NOT_SET);
}

void LoadCommand(int argumentsCount,char **argumentsArray, int position)
{
    if(argumentsCount < 6) // <command> není zadán
        Error(COMMAND_NOT_SET);
    if ((strcmp(argumentsArray[position],"boards") == 0) && (argumentsCount == 6)) // Jedná se o <command> - boards
    {
        COMMAND = BOARDS;
        return;
    }
    if(argumentsCount > 7) // SIGSEG ochrana
    {
        if((strcmp(argumentsArray[position],"board") == 0) && (strcmp(argumentsArray[position+1],"list") == 0) && (argumentsCount == 8))  // Jedná se o <command> - boards list <name>
        {
            NAME = argumentsArray[position+2]; // <name>
            COMMAND = BOARD_LIST;
            return;
        }
        if((strcmp(argumentsArray[position],"board") == 0) && (strcmp(argumentsArray[position+1],"add") == 0) && (argumentsCount == 8)) // Jedná se o <command> - board add <name>
        {
            NAME = argumentsArray[position+2]; // <name>
            COMMAND = BOARD_ADD;
            return;
        }
        if((strcmp(argumentsArray[position],"board") == 0) && (strcmp(argumentsArray[position+1],"delete") == 0) && (argumentsCount == 8)) // Jedná se o <command> - board del <name>
        {
            NAME = argumentsArray[position+2]; // <name>
            COMMAND = BOARD_DELETE;
            return;
        }
        if((strcmp(argumentsArray[position],"item") == 0) && (strcmp(argumentsArray[position+1],"add") == 0) && (argumentsCount >= 8)) // Jedná se o <command> - item add <name> <content>
        {
            NAME = argumentsArray[position+2]; // <name>
            COMMAND = ITEM_ADD;
            if(argumentsCount > 8) // Pokud byl zadán nějaký obsah, pro podporu i prázdného obsahu
                CONTENT = argumentsArray[position+3];
            return;
        }
        if(argumentsCount > 8) // SIGSEG ochrana
        {
            if((strcmp(argumentsArray[position],"item") == 0) && (strcmp(argumentsArray[position+1],"delete") == 0) && (argumentsCount == 9))
            {
                NAME = argumentsArray[position+2]; // <name>
                VerifyId(argumentsArray,position+3); // Pokud je <id> korektní, tak se nastaví, jak program spatně na chybě 8
                COMMAND = ITEM_DELETE;
                return;

            }
            if((strcmp(argumentsArray[position],"item") == 0) && (strcmp(argumentsArray[position+1],"update") == 0) && (argumentsCount >= 9))
            {
                NAME = argumentsArray[position+2]; // <name>
                VerifyId(argumentsArray,position+3); // Pokud je <id> korektní, tak se nastaví, jak program spatně na chybě 8
                if(argumentsCount > 9) // Pokud byl zadán nějaký obsah, pro podporu i prázdného obsahu
                    CONTENT = argumentsArray[position+4];
                COMMAND = ITEM_UPDATE;
                return;
            }
        }
    }
    COMMAND = UNKNOWN; // Nebyl zadán známý <command> nebo není ve správném formátu
}

void ParseArguments(int argumentsCount, char **argumentsArray)
{
    if(argumentsCount < 2) // Program nebyl spuštěn s žádným argumentem
        Error(MISSING_ARGUMENT);

    if(strcmp(argumentsArray[1],"-h") == 0) // Program byl spuštěn s parametrem -h a má se vypsat nápověda
    {
        if(argumentsCount == 2)
            PrintHelp();
        else
            Error(TOO_MANY_ARGUMENTS);
    }

    // Nastavení hodnoty <host> a <port>
     LoadPort(argumentsCount,argumentsArray); // -p <cislo_portu> (podporuji moznost prohozeni <port> a <host>
     LoadHost(argumentsCount,argumentsArray); // -p <cislo_portu> -H <hostname>
     SetCheck();

    if (argumentsCount >= 5) // Ověření aby nemohl nastat SIGSEG
        LoadCommand(argumentsCount,argumentsArray,5); // -p <cislo_portu> -H <hostname> <command>

    if (COMMAND == UNKNOWN) // Nebyl zadán žádný <command> nebo nebyl ve správném formátu
        Error(INVALID_COMMAND);
}

string CreateRequest()
{
    stringstream bufferStream;
    string buffer;

    switch (COMMAND)
    {
        case BOARDS:
            bufferStream << "GET /boards HTTP/1.1\r\n";
            bufferStream << "Host: " << HOST << ":" << PORT << "\r\n\r\n";
            break;
        case BOARD_ADD:
            bufferStream << "POST /boards/" << NAME << " HTTP/1.1\r\n";
            bufferStream << "Host: " << HOST << ":" << PORT << "\r\n\r\n";
            break;
        case BOARD_DELETE:
            bufferStream << "DELETE /boards/" << NAME << " HTTP/1.1\r\n";
            bufferStream << "Host: " << HOST << ":" << PORT << "\r\n\r\n";
            break;
        case BOARD_LIST:
            bufferStream << "GET /board/" << NAME << " HTTP/1.1\r\n";
            bufferStream << "Host: " << HOST << ":" << PORT << "\r\n\r\n";
            break;
        case ITEM_ADD:
            bufferStream << "POST /board/" << NAME << " HTTP/1.1\r\n";
            bufferStream << "Host: " << HOST << ":" << PORT << "\r\n";
            bufferStream << "Content-Type: text/plain\r\n";
            bufferStream << "Content-Length: " << CONTENT.length() << "\r\n\r\n";
            bufferStream << CONTENT;
            break;
        case ITEM_DELETE:
            bufferStream << "DELETE /board/" << NAME << "/" << ID << " HTTP/1.1\r\n";
            bufferStream << "Host: " << HOST << ":" << PORT << "\r\n\r\n";
            break;
        case ITEM_UPDATE :
            bufferStream << "PUT /board/" << NAME << "/" << ID <<" HTTP/1.1\r\n";
            bufferStream << "Host: " << HOST << ":" << PORT << "\r\n";
            bufferStream << "Content-Type: text/plain\r\n";
            bufferStream << "Content-Length: " << CONTENT.length() << "\r\n\r\n";
            bufferStream << CONTENT;
            break;
        case UNKNOWN:
            break;    
    }

    buffer = bufferStream.str();
    return buffer;
}

void PrintResponse(string response)
{
    // Zjištění délky části která se má vypsat na stdout (tělo HTTP)
    regex contentLenRegex ("Content-Length: (\\d+)");
    smatch matchesContentLen;

    if(regex_search(response,matchesContentLen,contentLenRegex))
    {
        int contentLen;
        istringstream contentLenStream (matchesContentLen[1]);
        contentLenStream >> contentLen;

        if(contentLen > 0)
        {
            string content;
            string header;
            content = response.substr(response.length() - contentLen);
            cerr << header; // stderr výpis (HTTP hlavička)
            cout << content; // stdout výpis (HTTP tělo)
            header = response.substr(0,response.length() - contentLen);
            return; // Vyskočím ven abych na stderr nevypisoval 2x
        }
    }
    cerr << response; // stderr výpis (HTTP hlavička)
    return;
}

int ConnectToServer()
{

    struct hostent *serverAdrress; // Ukazatel na adresy serveru
    struct sockaddr_in client; // Klient
    struct sockaddr_in server; // Server
    int clientSocket;

    string request;

    char bufferResponse[BUFFER_SIZE];


    memset(&server,0,sizeof(server)); // vynulování
    memset(&client,0,sizeof(client)); // // vynulování

    server.sin_family = AF_INET; // IPv4 na serveru

    // DNS rozpoznání HOST pomocí gethostbyname ()
    if((serverAdrress = gethostbyname(HOST.c_str()))== nullptr) // Nemám žádnou adresu
        Error(NO_SERVER_INFO);

    memcpy(&server.sin_addr,serverAdrress->h_addr,serverAdrress->h_length); // Adresa na serveru
    server.sin_port = htons(PORT); // Port na serveru, musí to být v network byte orderu kvůli kompabilitě

    if((clientSocket = socket(AF_INET,SOCK_STREAM,0)) == FAIL) // Otevření socketu klienta
        Error(SOCKET_CREATE_ERROR);

    if (connect(clientSocket, (struct sockaddr *)&server, sizeof(server)) == FAIL)
        Error(CONNECT_TO_SERVER_ERROR);


    request = CreateRequest(); // Vytvoření požadavku na server

    send(clientSocket,request.c_str(), request.size(),0); // Odešlu požadavek  na server
    recv(clientSocket, bufferResponse, BUFFER_SIZE, 0); // Očekávám odpověď od serveru

    string response (bufferResponse);
    if(response.empty())
        return FAIL; // Server neodeslal odpověď můj požadavek byl pravděpodovně zahozen

    PrintResponse(response);
    close(clientSocket);
    return OK;
}

int main(int argc,char **argv)
{
    ParseArguments(argc,argv);
    return ConnectToServer();
}

/* end isaclient.cpp */
