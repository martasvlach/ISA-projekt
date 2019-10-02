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

int main(int argc, char **argv)
{
    ParseArguments(argc,argv);
    if(DEBUG)
        DEBUG_USERINPUT();
    return OK;
}

/* end isaserver.cpp */