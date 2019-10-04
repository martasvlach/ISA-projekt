/************************************************
 * HTTP nástěnka v C/C++
 * Autor: Martin Vlach [xvlach18@stud.fit.vutbr.cz]
 * Předmět: ISA - Síťové aplikace a správa sítí
 * Soubor: isaserver.h
 ************************************************/

#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <regex>

// Síťové knihovny
#include <netdb.h>
#include <sys/socket.h>

using namespace std;

// Port specifikovaný uživatel, na kterém bude server očekávat spojení
int PORT;

// Návratový kód programu
int RETURN_CODE;

// Funkce pro zpracování programových argumentů zadaných uživatelem
void ParseArguments(int arrayLen, char **argumentsArray);

// Funkce pro výpis uživatelské nápovědy
void PrintHelp();

// Funkce pro výpis chzby a ukončení s patričným návratovým kódem
void Error(int errorCode);

// Kontrola zda je zadaný řetězec validním číslem portu (<0;65535>)
void IsLegitPortNumber(char *pn);

// DEBUG FLAG
bool DEBUG = true;

// Funkce pro načtění struktury informace o IP adrese nutné pro otevření socketu
struct addrinfo *LoadAddressInfo();

// Funkce pro kontrolu, že se mi podařilo získat potřebné infomace o adrese
void CheckAddressInfo(struct addrinfo *addressInfo);

// Funkce pro nastavení a zapnutí serveru
void ServerStart();

// Hlavní běhová funkce serveru
void ServerRun();

#define OK 0
#define FAIL -1

/*
 * CHYBY
 */


// Chyby zpracování argumentů

#define UNKNOWN_ARGUMENT 1
#define TOO_MANY_ARGUMENTS 2
#define TOO_FEW_ARGUMENTS 3
#define PORT_NUM_UNSPECIFIED 4
#define PORT_BAD_RANGE 5

// Chyby ohledně síťové složky

#define NULL_ADDRESS_INFO 6
#define SOCKET_ERROR 7

/* end isaserver.h */

/* DEBUG */

void DEBUG_USERINPUT()
{
    stringstream ss;
    ss << PORT;
    string portString = ss.str();

    cout << "Port : " + portString << endl;
}

/* END DEBUG */
