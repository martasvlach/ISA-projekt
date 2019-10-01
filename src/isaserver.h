/************************************************
 * HTTP nástěnka v C/C++
 * Autor: Martin Vlach [xvlach18@stud.fit.vutbr.cz]
 * Předmět: ISA - Síťové aplikace a správa sítí
 * Soubor: isaserver.h
 ************************************************/

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <regex>


using namespace std;

// Ukončení bez chyby
#define OK 0

// Neznámý argument
#define UNKNOWN_ARGUMENT 1

// Příliš mnoho argumentů
#define TOO_MANY_ARGUMENTS 2

// Nebyl zadán žádný argument
#define TOO_FEW_ARGUMENTS 3

// Nebylo zadání číslo portu
#define PORT_NUM_UNSPECIFIED 4

// Zadaný port není v korektním rozsahu <0;65535>
#define PORT_BAD_RANGE 5

// Port specifikovaný uživatel, na kterém bude server očekávat spojení
int LISTEN_PORT = -1;

// Návratový kód programu
int RETURN_CODE = OK;

// Funkce pro zparsování uživatelských argumentů
// char **arguments_array - pole uživatelských argumentů
void ParseArguments(int arrayLen, char **argumentsArray);

// Funkce pro výpis uživatelské nápovědy
void PrintHelp();

// Chybové výpisy a nastavení RC chyby
// errorCode - kód chyby
void Error(int errorCode);

// Kontrola zda je zadaný řetězec validním číslem portu
// char *pn - řetězec s očekávaným číslem portu
bool IsLegitPortNum(char *pn);

/* end isaserver.h */