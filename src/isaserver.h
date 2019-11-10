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
#include <thread>
#include <mutex>

// Knihovny pro práci se síťovou složkou projektu
#include <netdb.h>
#include <sys/socket.h>

using namespace std;

// Schéma pro uložení jednolitvých nástěnek a příspěvků na těchto nástěnkách
vector<vector<string>> BOARDS;

/* Zámek pro zamčení schématu uložení tabulek při přístupu k totumu schématu ve vláknu programu
 * Důvodem je konzistence dat, a jejich aktuálnost po dobu zpracování požadavku
*/
mutex mutexBOARDS;

// Port specifikovaný uživatel, na kterém bude server očekávat spojení
int PORT;

// Návratový kód programu
int RETURN_CODE;

// DEBUG FLAG
bool DEBUG = false;

// Funkce pro zpracování programových argumentů zadaných uživatelem
void ParseArguments(int arrayLen, char **argumentsArray);

// Funkce pro výpis uživatelské nápovědy
void PrintHelp();

// Funkce pro výpis chzby a ukončení s patričným návratovým kódem
void Error(int errorCode);

// Kontrola zda je zadaný řetězec validním číslem portu (<0;65535>)
void IsLegitPortNumber(char *pn);

// Funkce pro načtění struktury informace o IP adrese nutné pro otevření socketu
struct addrinfo *LoadAddressInfo();

// Funkce pro kontrolu, že se mi podařilo získat potřebné infomace o adrese
void CheckAddressInfo(struct addrinfo *addressInfo);

// Funkce pro nastavení a zapnutí serveru
int ServerStart();

// Hlavní běhová funkce serveru
int ServerRun(int serverFD);

// Zpracování požadavku na server
void RequestResolver(int ClientSocket);

// Velikost bufferu pro posílání po síťi
// Velikost "odvozena" z diskuze + je dostatečně velká pro normální rozumné použití aplikace tohoto typu
// https://stackoverflow.com/questions/2862071/how-large-should-my-recv-buffer-be-when-calling-recv-in-the-socket-library
#define BUFFER_SIZE 65536

// Fronta pro příchozí spojení
#define QUEUE_LEN 5

// Úspěšnost
#define OK 0

// Neúspěch
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
#define QUEUE_ERROR 8
#define ACCEPT_ERROR 9

/* end isaserver.h */
