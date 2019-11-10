/************************************************
 * HTTP nástěnka v C/C++
 * Autor: Martin Vlach [xvlach18@stud.fit.vutbr.cz]
 * Předmět: ISA - Síťové aplikace a správa sítí
 * Soubor: isaclient.h
 ************************************************/

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <regex>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;

/*
 * Požadavky klienta
 * 1) boards - Vrátí uživateli seznam všech dostupných nástěnek (1/řádek)
 * 2) board add <name> - Vytvoří na serveru novou nástěnku s názvem <name>
 * 3) board delete <name> - Smaže ze serveru nástěnku s názvem <name> a všechen její obsah
 * 4) boards list <name> - Vrátí obsah z dané nástěnky se jménem <name> (jeden příspěvek / řádek)
 * 5) item add <name> <content> - Přidá na nástěnku s názvem <name> příspěvek <content> (Pokud není zadán content, odešle se prázdný)
 * 6) item delete <name> <id> - Odstraní z nástěnky se jménem <name> příspěvek s číslem (pořadím) <id>
 * 7) item update <name> <id> - Aktualizuje na nástěnce se jménem <name> příspěvek s číslem (pořadím) <id>
 */

/*
 * Počáteční magická konstanta pro int
 */

#define INT_MAGIC -42

// Uživatelem zadané ID příspěvku
int ID = INT_MAGIC;

// Uživatelem zadané číslo portu
int PORT = INT_MAGIC;

// velikost BUFFERU pro příjem odpovědi od serveru
// Velikost "odvozena" z diskuze + je dostatečně velká pro normální použití aplikace tohoto typu
// https://stackoverflow.com/questions/2862071/how-large-should-my-recv-buffer-be-when-calling-recv-in-the-socket-library
#define BUFFER_SIZE 65536

// Uživatelem zadaný HOST
string HOST;

enum commandEnum
{
    BOARDS, // 0
    BOARD_ADD, // 1
    BOARD_DELETE, // 2
    BOARD_LIST, // 3
    ITEM_ADD, // 4
    ITEM_DELETE, // 5
    ITEM_UPDATE, // 6
    UNKNOWN // 7
};

// Uživatelem zvolený požadavek
commandEnum COMMAND = UNKNOWN;

// Uživatelem definovaný obsah příspěvku (pokud nezadá >> Content-Lenght = 0)
string CONTENT;

// Uživatelem zvolené jméno nástěnky
string NAME;

// Návratový kód programu
int RETURN_CODE;

// Funkce pro výpis uživatelské nápovědy
void PrintHelp();

// Funkce pro zpracování uživatelských argumentů
void ParseArguments(int argumentsCount, char **argumentsArray);

// Funkce pro chybový výpis a nastavení chybového návratového kódu
void Error(int errorCode);

// Funkce pro načtení parametru <host>
void LoadHost(int argumentsCount, char **argumentsArray);

// Funkce pro načtení parametru <port>
void LoadPort(int argumentsCount, char **argumentsArray);

// Funkce na ověření, že zadaný port je číslo z rozsahu <0;65535>
bool CheckPortNumber(char *pn);

// Kontrola zda je nastavený parametr <host> a <port>
void SetCheck();

// Funkce pro načtení <command> zadaného uživatelem
void LoadCommand(int argumentsCount,char **argumentsArray, int position);

//Funkce pro kontrolu uživatelem zadaného <id> u <command>
void VerifyId(char **array,int position);

// Výpis hlavičky HTTP odpovědi na stderr a těla odpovědi na stdout
void PrintResponse(string response);

// Ukazatel na adresy serveru
struct hostent *serverAdrress;


// Funkce pro výpis těla odpovědi na stdout a hlavičky odpovědi na stderr
void PrintResponse(string response);

#define OK 0
#define FAIL -1

/*
 * CHYBY
 */

#define MISSING_ARGUMENT 1
#define TOO_MANY_ARGUMENTS 2
#define HOST_NOT_SET 3
#define PORT_NOT_SET 4
#define INVALID_PORT_NUMBER 5
#define COMMAND_NOT_SET 6
#define INVALID_COMMAND 7
#define INVALID_ID_NUMBER 8

#define CONNECT_TO_SERVER_ERROR 9
#define NO_SERVER_INFO 10
#define SOCKET_CREATE_ERROR 11

/* end isaclient.h */
