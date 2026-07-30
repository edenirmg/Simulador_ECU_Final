/**
 * ====
 * MenuData.h - Declaração da Estrutura dos Menus
 * ====
 * 
 * Declara todos os arrays de MenuItem (submenus) e a tabela de títulos.
 * As definições reais estão em MenuData.cpp.
 * 
 * Estrutura hierárquica dos menus:
 *   menuPrincipal
 *   ├── Carros (submenuCarros) — ordem mercado BR
 *   │   ├── VW/Audi/Seat → módulos IAW/ME/MP9/Simos
 *   │   ├── Fiat → IAW 1G7/G7 / 4AFB / ME7.9.9 / 59FB…
 *   │   ├── GM/Chevrolet → Multec / Motronic / ME7.9.9
 *   │   ├── Ford → EEC-V/VI / IAW 4CFR
 *   │   ├── Renault → Sirius / Fenix / IAW 5NR
 *   │   ├── Peugeot / Citroën → ME7.4.x / IAW 5NP/6LP/8P
 *   │   ├── Honda → PGM-FI
 *   │   ├── Toyota [em desenvolvimento]
 *   │   ├── BMW / Chrysler
 *   ├── Diesel Leve — VW/GM/Ford/Fiat/PSA/MB/Nissan/Toyota/Troller/Iveco
 *   │   └── módulos EDC15/16/17 · SID901/208 · Denso (60-2, sem fase)
 *   ├── Diesel Pesado — MB/Scania/Iveco/Volvo/Cummins/MWM/MAN
 *   │   └── PLD · EMS S6 · MS6.x · EDC7 · D12/D13 (60-2, sem fase)
 *   ├── Sinais Básicos
 *   └── Ajustes
 * ====
 */

#ifndef MENU_DATA_H
#define MENU_DATA_H

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "Tipos.h"

// ====
// DECLARAÇÃO DOS MENUS (extern - definidos em MenuData.cpp)
// ====

extern const MenuItem menuPrincipal[] PROGMEM;
extern const MenuItem subMenuSinaisBasicos[] PROGMEM;
extern const MenuItem submenuCarros[] PROGMEM;
extern const MenuItem submenuAudi[] PROGMEM;
extern const MenuItem submenuBmw[] PROGMEM;
extern const MenuItem submenuChrysler[] PROGMEM;
extern const MenuItem submenuCitroen[] PROGMEM;
extern const MenuItem submenuFiat[] PROGMEM;
extern const MenuItem submenuFord[] PROGMEM;
extern const MenuItem submenuGm[] PROGMEM;
extern const MenuItem submenuHonda[] PROGMEM;
extern const MenuItem submenuPeugeot[] PROGMEM;
extern const MenuItem submenuRenault[] PROGMEM;
extern const MenuItem submenuDieselLeve[] PROGMEM;
extern const MenuItem submenuDlVw[] PROGMEM;
extern const MenuItem submenuDlGm[] PROGMEM;
extern const MenuItem submenuDlFord[] PROGMEM;
extern const MenuItem submenuDlFiat[] PROGMEM;
extern const MenuItem submenuDlPsa[] PROGMEM;
extern const MenuItem submenuDlMb[] PROGMEM;
extern const MenuItem submenuDlNissan[] PROGMEM;
extern const MenuItem submenuDlToyota[] PROGMEM;
extern const MenuItem submenuDlTroller[] PROGMEM;
extern const MenuItem submenuDlIveco[] PROGMEM;
extern const MenuItem submenuDieselPesado[] PROGMEM;
extern const MenuItem submenuDpMb[] PROGMEM;
extern const MenuItem submenuDpScania[] PROGMEM;
extern const MenuItem submenuDpIveco[] PROGMEM;
extern const MenuItem submenuDpVolvo[] PROGMEM;
extern const MenuItem submenuDpCummins[] PROGMEM;
extern const MenuItem submenuDpMwm[] PROGMEM;
extern const MenuItem submenuDpMan[] PROGMEM;
extern const MenuItem submenuHallMagnetico[] PROGMEM;
extern const MenuItem submenu5x12Volts[] PROGMEM;
extern const MenuItem submenuAjustes[] PROGMEM;

// Tabela de títulos dos menus
extern const MenuTitulo titulosMenus[] PROGMEM;
extern const uint8_t titulosMenusCount; // Quantidade de entradas em titulosMenus, calculada em MenuData.cpp para evitar sizeof em array incompleto

// ====
// FUNÇÕES UTILITÁRIAS DE MENU
// ====

/**
 * Conta o número de itens em um menu (até encontrar MENU_END).
 * @param menu Ponteiro para o array de MenuItem (PROGMEM)
 * @return Número de itens (sem contar o terminador)
 */
uint8_t contarItens(const MenuItem *menu);

/**
 * Lê o texto de um item de menu da PROGMEM para um buffer RAM.
 * @param itemPtr Ponteiro para o MenuItem em PROGMEM
 * @param buffer  Buffer de destino na RAM
 * @param bufSize Tamanho do buffer
 * @return Ponteiro para o buffer preenchido
 */
const char *readMenuItemTexto(const MenuItem *itemPtr, char *buffer, size_t bufSize);

/**
 * Lê um MenuItem completo da PROGMEM para a RAM.
 * @param itemPtr Ponteiro para o MenuItem em PROGMEM
 * @return Cópia do MenuItem na RAM
 */
MenuItem readMenuItem(const MenuItem *itemPtr);

#endif // MENU_DATA_H