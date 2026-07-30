/**
 * ============================================================================
 * MenuTextos.h - Strings de Texto dos Menus (PROGMEM)
 * ============================================================================
 * 
 * Centraliza TODAS as strings de texto utilizadas nos menus do sistema.
 * Todos os textos são armazenados em PROGMEM para economizar RAM.
 * 
 * Organização:
 *   - Títulos dos menus (exibidos na linha 0 do LCD)
 *   - Textos do menu principal
 *   - Textos dos submenus por fabricante
 *   - Textos dos menus de configuração (Hall/Magnético, 5V/12V)
 * 
 * COMO ADICIONAR TEXTOS PARA UM NOVO VEÍCULO:
 *   1. Declare a string com PROGMEM neste arquivo
 *   2. Use o prefixo "txt" para itens de menu (ex: txtNomeVeiculo)
 *   3. Use o prefixo "titulo" para títulos de menu (ex: tituloMarca)
 *   4. Adicione a referência no MenuData.cpp
 * ============================================================================
 */

#ifndef MENU_TEXTOS_H
#define MENU_TEXTOS_H

#include <avr/pgmspace.h>

// ============================================================================
// TÍTULOS DOS MENUS (linha 0 do LCD, centralizado)
// ============================================================================
extern const char tituloPrincipal[] PROGMEM;
extern const char tituloSinaisBasicos[] PROGMEM;
extern const char tituloCarros[] PROGMEM;
extern const char tituloAudi[] PROGMEM;
extern const char tituloBmw[] PROGMEM;
extern const char tituloFiat[] PROGMEM;
extern const char tituloChrysler[] PROGMEM;
extern const char tituloCitroen[] PROGMEM;
extern const char tituloFord[] PROGMEM;
extern const char tituloGm[] PROGMEM;
extern const char tituloHonda[] PROGMEM;
extern const char tituloPeugeot[] PROGMEM;
extern const char tituloRenault[] PROGMEM;
extern const char tituloHallMagnetico[] PROGMEM;
extern const char titulo5x12Volts[] PROGMEM;

// ============================================================================
// MENU PRINCIPAL
// ============================================================================
extern const char txtCarros[] PROGMEM;
extern const char txtDieselLeve[] PROGMEM;
extern const char txtDieselPesado[] PROGMEM;
extern const char txtSinaisBasicos[] PROGMEM;

// ============================================================================
// SINAIS BÁSICOS
// ============================================================================
extern const char txtHall[] PROGMEM;
extern const char txt36x1[] PROGMEM;
extern const char txt36x2[] PROGMEM;
extern const char txt44x4[] PROGMEM;
extern const char txt60x2[] PROGMEM;

// ============================================================================
// SUBMENU CARROS - Marcas
// ============================================================================
extern const char txtAudiSeatVw[] PROGMEM;
extern const char txtBmw[] PROGMEM;
extern const char txtCrysler[] PROGMEM;
extern const char txtCitroen[] PROGMEM;
extern const char txtFiat[] PROGMEM;
extern const char txtFord[] PROGMEM;
extern const char txtGM[] PROGMEM;
extern const char txtHonda[] PROGMEM;
extern const char txtPeugeot[] PROGMEM;
extern const char txtRenault[] PROGMEM;
extern const char txtToyota[] PROGMEM;

// ============================================================================
// AUDI / SEAT / VW
// ============================================================================
extern const char txtAudiTfsi[] PROGMEM;
extern const char txtAudiAtf[] PROGMEM;
extern const char txt1AvpAvb[] PROGMEM;
extern const char txt1AvsAvi[] PROGMEM;
extern const char txt4bvAvp[] PROGMEM;
extern const char txt4Gv[] PROGMEM;
extern const char txt4lv[] PROGMEM;
extern const char txt5Wp4[] PROGMEM;
extern const char txtMp9[] PROGMEM;
extern const char txtMe7x[] PROGMEM;
extern const char txtSimos[] PROGMEM;

// ============================================================================
// BMW
// ============================================================================
extern const char txt320i[] PROGMEM;
extern const char txtX1[] PROGMEM;
extern const char txtM3[] PROGMEM;

// ============================================================================
// CHRYSLER
// ============================================================================
extern const char txtChrys1[] PROGMEM;
extern const char txtChrys2[] PROGMEM;

// ============================================================================
// CITROËN
// ============================================================================
extern const char txtIaw8p[] PROGMEM;
extern const char txtMe72[] PROGMEM;
extern const char txtMe74[] PROGMEM;
extern const char txtMa3[] PROGMEM;
extern const char txtSagen[] PROGMEM;
extern const char txt6pl1[] PROGMEM;
extern const char txtMareli8[] PROGMEM;

// ============================================================================
// FIAT
// ============================================================================
extern const char txt1g7[] PROGMEM;
extern const char txtEtorq[] PROGMEM;
extern const char txtFire[] PROGMEM;
extern const char txtEvo[] PROGMEM;
extern const char txtMarea[] PROGMEM;
extern const char txtFirefly[] PROGMEM;
extern const char txtFiat500[] PROGMEM;

// ============================================================================
// FORD
// ============================================================================
extern const char txtDuratec20[] PROGMEM;
extern const char txtDuratec23[] PROGMEM;
extern const char txtTiVct[] PROGMEM;
extern const char txtZetec[] PROGMEM;

// ============================================================================
// GENERAL MOTORS (GM)
// ============================================================================
extern const char txtBoschM15[] PROGMEM;
extern const char txtEconoflex[] PROGMEM;
extern const char txtFlexpower[] PROGMEM;
extern const char txtMotronic15[] PROGMEM;
extern const char txtMultecVhc[] PROGMEM;

// ============================================================================
// HONDA
// ============================================================================
extern const char txtD16w7[] PROGMEM;
extern const char txtMotorL13a[] PROGMEM;
extern const char txtMotorIvetc[] PROGMEM;

// ============================================================================
// PEUGEOT
// ============================================================================
extern const char txtEw10a[] PROGMEM;
extern const char txt5Np1[] PROGMEM;
extern const char txtSagem2306[] PROGMEM;
extern const char txtMe7416[] PROGMEM;
extern const char txtMe7216[] PROGMEM;

// ============================================================================
// RENAULT
// ============================================================================
extern const char txtF4rk4r[] PROGMEM;
extern const char txtFenix3[] PROGMEM;
extern const char txtk7m[] PROGMEM;
extern const char txt5nr1[] PROGMEM;
extern const char txtSagen3[] PROGMEM;
extern const char txtSirius32[] PROGMEM;
extern const char txtSirius3134[] PROGMEM;

// ============================================================================
// HALL/MAGNÉTICO E TENSÃO
// ============================================================================
extern const char txtSinalHall[] PROGMEM;
extern const char txtMagnetico[] PROGMEM;
extern const char txt12V[] PROGMEM;
extern const char txt5V[] PROGMEM;

#endif // MENU_TEXTOS_H
