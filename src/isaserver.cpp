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
    cout << "Příklad spuštění -- ./isaserver -p 5777" << endl;
    cout << "Kde -p je povinný parametr který značí číslo portu na kterém má server očekávat spojení" << endl;
    cout << "Toto číslo musí být validní číslo portu (<0;65535>)" << endl;
    cout << "=============================================" << endl;
}

void Error(int errorCode)
{
    switch (errorCode)
    {
        case TOO_FEW_ARGUMENTS:
            cerr << "Nebyl zadán povinný uživatelský argument" << endl;
            RETURN_CODE = TOO_FEW_ARGUMENTS;
            break;
        case TOO_MANY_ARGUMENTS:
            cerr << "Bylo zadáno příliš mnoho argumentů" << endl;
            RETURN_CODE = TOO_MANY_ARGUMENTS;
            break;
        case PORT_NUM_UNSPECIFIED:
            cerr << "Nebylo zadáno číslo portu na kterém má server očekávat spojení" << endl;
            RETURN_CODE = PORT_NUM_UNSPECIFIED;
            break;
        case UNKNOWN_ARGUMENT:
            cerr << "Zadaný argument nebyl rozpoznán" << endl;
            RETURN_CODE = UNKNOWN_ARGUMENT;
            break;
        case PORT_BAD_RANGE:
            cerr << "Zadaný port není číslem v rozsahu <0;65535>" << endl;
            RETURN_CODE = PORT_BAD_RANGE;
            break;
    }
}

bool IsLegitPortNum(char *pn)
{
    string portNum (pn);
    regex intRegex ("^([0-9]+)$");
    smatch matches;

    if (regex_search(portNum,matches,intRegex)) // Kontrola zda se vůbec jedná o číslo
    {
        string::size_type st;
        LISTEN_PORT = stoi(portNum,&st);

        if (LISTEN_PORT >= 0 && LISTEN_PORT <= 65535)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

void ParseArguments(int arrayLen, char **argumentsArray)
{
    if (arrayLen < 2)
    {
        Error(TOO_FEW_ARGUMENTS);
        return;
    }

    if (strcmp(argumentsArray[1],"-h") == 0)
    {
        if(arrayLen != 2)
        {
            Error(TOO_MANY_ARGUMENTS);
            return;
        }
        else
        {
            PrintHelp();
            return;
        }
    }
    else if (strcmp(argumentsArray[1],"-p") == 0)
    {
        if(arrayLen < 3)
        {
            Error(PORT_NUM_UNSPECIFIED);
            return;
        }
        if(arrayLen > 3)
        {
            Error(TOO_MANY_ARGUMENTS);
            return;
        }

        if(!IsLegitPortNum(argumentsArray[2]))
        {
            Error(PORT_BAD_RANGE);
            return;
        }
    }
    else
    {
        Error(UNKNOWN_ARGUMENT);
        return;
    }
}

int main(int argc, char **argv)
{
    ParseArguments(argc,argv);
    return RETURN_CODE;
}

/* end isaserver.cpp */