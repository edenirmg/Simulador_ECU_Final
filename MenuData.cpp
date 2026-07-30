/**
 * ====
 * MenuData.cpp - Definição da Estrutura Completa dos Menus
 * ====
 * 
 * Este arquivo contém:
 *   - Definição de todas as strings PROGMEM (títulos e textos de itens)
 *   - Definição de todos os arrays de MenuItem (PROGMEM)
 *   - Tabela de associação menu → título
 *   - Funções utilitárias para leitura de menus da PROGMEM
 * 
 * COMO ADICIONAR UM NOVO VEÍCULO AO MENU:
 * ----
 * 1. Adicione a string PROGMEM na seção apropriada do fabricante:
 *      const char txtNovo[] PROGMEM = "Novo Modelo 1.6";
 * 
 * 2. Adicione o MENU_ITEM no submenu do fabricante:
 *      MENU_ITEM(NULL, txtNovo, &SelecionaSin, XX),  // XX = índice na tabela sinais[]
 *    Ou, se precisar de submenu (ex: Hall/Magnético):
 *      MENU_ITEM(submenuHallMagnetico, txtNovo, NULL, XX),
 * 
 * 3. Certifique-se de que o índice XX corresponde à posição correta em Sinais.cpp
 * 
 * COMO ADICIONAR UMA NOVA MARCA:
 * ----
 * 1. Crie as strings PROGMEM (título + itens)
 * 2. Crie o array submenuNovaMarca[] PROGMEM
 * 3. Adicione entrada em submenuCarros[]
 * 4. Adicione entrada em titulosMenus[]
 * 5. Declare extern no MenuData.h
 * ====
 */

#include "MenuData.h"
#include "MenuTextos.h"
#include "Sinais.h"
#include "Reles.h"
#include "Ajustes.h"

// Declaração externa da função de ativação do monitor
extern void AtivaMonitorAnalogico(uint8_t param);

// ====
// DEFINIÇÃO DAS STRINGS PROGMEM
// ====

// --- Títulos dos menus ---
const char tituloPrincipal[] PROGMEM     = "Menu Principal";
const char tituloSinaisBasicos[] PROGMEM = "Sinais Basicos";
const char tituloCarros[] PROGMEM        = "Carros";
const char tituloAudi[] PROGMEM          = "VW/Audi/Seat";
const char tituloBmw[] PROGMEM           = "BMW";
const char tituloFiat[] PROGMEM          = "Fiat";
const char tituloChrysler[] PROGMEM      = "Chrysler";
const char tituloCitroen[] PROGMEM       = "Citroen";
const char tituloFord[] PROGMEM          = "Ford";
const char tituloGm[] PROGMEM            = "GM/Chevrolet";
const char tituloHonda[] PROGMEM         = "Honda";
const char tituloPeugeot[] PROGMEM       = "Peugeot";
const char tituloRenault[] PROGMEM       = "Renault";
const char tituloDieselLeve[] PROGMEM    = "Diesel Leve";
const char tituloDlVw[] PROGMEM          = "VW Diesel";
const char tituloDlGm[] PROGMEM          = "GM Diesel";
const char tituloDlFord[] PROGMEM        = "Ford Diesel";
const char tituloDlFiat[] PROGMEM        = "Fiat Diesel";
const char tituloDlPsa[] PROGMEM         = "PSA Diesel";
const char tituloDlMb[] PROGMEM          = "MB Diesel";
const char tituloDlNissan[] PROGMEM      = "Nissan Diesel";
const char tituloDlToyota[] PROGMEM      = "Toyota Diesel";
const char tituloDlTroller[] PROGMEM     = "Troller Diesel";
const char tituloDlIveco[] PROGMEM       = "Iveco Diesel";
const char tituloDieselPesado[] PROGMEM  = "Diesel Pesado";
const char tituloDpMb[] PROGMEM          = "MB Pesado";
const char tituloDpScania[] PROGMEM      = "Scania";
const char tituloDpIveco[] PROGMEM       = "Iveco Pesado";
const char tituloDpVolvo[] PROGMEM       = "Volvo";
const char tituloDpCummins[] PROGMEM     = "Cummins";
const char tituloDpMwm[] PROGMEM         = "MWM";
const char tituloDpMan[] PROGMEM         = "MAN/VW Cam.";
const char tituloHallMagnetico[] PROGMEM = "Saida de Sinal";
const char titulo5x12Volts[] PROGMEM     = "Tensão Hall";
const char tituloAjustes[] PROGMEM       = "Ajustes";

// --- Menu principal ---
const char txtCarros[] PROGMEM       = "Carros";
const char txtDieselLeve[] PROGMEM   = "Diesel Leve";
const char txtDieselPesado[] PROGMEM = "Diesel Pesado";
const char txtSinaisBasicos[] PROGMEM = "Sinais Basicos";
const char txtAjustes[] PROGMEM       = "Ajustes";

// --- Sinais básicos ---
const char txtHall[] PROGMEM = "Hall Continuo";
const char txt36x1[] PROGMEM = "36-1";
const char txt36x2[] PROGMEM = "36-2";
const char txt44x4[] PROGMEM = "44-4";
const char txt60x2[] PROGMEM = "60-2";

// --- Carros: Marcas (ordem mercado BR / ECU TEC) ---
const char txtAudiSeatVw[] PROGMEM = "VW/Audi/Seat";
const char txtFiat[] PROGMEM       = "Fiat";
const char txtGM[] PROGMEM         = "GM/Chevrolet";
const char txtFord[] PROGMEM       = "Ford";
const char txtRenault[] PROGMEM    = "Renault";
const char txtPeugeot[] PROGMEM    = "Peugeot";
const char txtCitroen[] PROGMEM    = "Citroen";
const char txtHonda[] PROGMEM      = "Honda";
const char txtToyota[] PROGMEM     = "Toyota (pendente)";
const char txtBmw[] PROGMEM        = "BMW";
const char txtCrysler[] PROGMEM    = "Chrysler";

// --- VW/Audi/Seat: código do módulo (estilo ECUPROG/ECU TEC) ---
const char txtAudiTfsi[] PROGMEM = "ME TFSI 1.8/2.0";
const char txtAudiAtf[] PROGMEM  = "ATF 2.0 8V";
const char txt1AvpAvb[] PROGMEM  = "IAW 1AVP/1AVB";
const char txt1AvsAvi[] PROGMEM  = "IAW 1AVS/1AVI";
const char txt4bvAvp[] PROGMEM   = "IAW 4BV/4AVP";
const char txt4Gv[] PROGMEM      = "IAW 4GV / ME";
const char txt4lv[] PROGMEM      = "IAW 4LV/4SV/4MV";
const char txt5Wp4[] PROGMEM     = "IAW 5WP4";
const char txtMp9[] PROGMEM      = "Bosch MP9.0";
const char txtMe7x[] PROGMEM     = "Bosch ME7.5.x";
const char txtSimos[] PROGMEM    = "Siemens Simos";

// --- BMW (veículo + tipico PCM; stubs de sinal) ---
const char txt320i[] PROGMEM = "MSD80 320i 2.0";
const char txtX1[] PROGMEM   = "MSD80 X1 2.0";
const char txtM3[] PROGMEM   = "MSS60 M3 V8";

// --- Chrysler ---
const char txtChrys1[] PROGMEM = "SBEC 1.8/2.0";
const char txtChrys2[] PROGMEM = "SBEC 2.2/2.5";

// --- Citroën (módulos PSA / franceses) ---
const char txtIaw8p[] PROGMEM   = "IAW 8P/1AP/48P";
const char txtMe72[] PROGMEM    = "Bosch ME7.2/7.3";
const char txtMe74[] PROGMEM    = "Bosch ME7.4.4";
const char txtMa3[] PROGMEM     = "MA3.0/3.1/5.1";
const char txtSagen[] PROGMEM   = "Sagem SL96/2000";
const char txt6pl1[] PROGMEM    = "IAW 6LP1/6LPB";
const char txtMareli8[] PROGMEM = "IAW 8P20";

// --- Fiat (módulo + família motor) ---
const char txt1g7[] PROGMEM     = "IAW 1G7/G7";
const char txtEtorq[] PROGMEM   = "ME7.9.9 Etorq";
const char txtFire[] PROGMEM    = "IAW 4AFB Fire";
const char txtEvo[] PROGMEM     = "IAW 7GF Evo";
const char txtMarea[] PROGMEM   = "IAW 59FB Marea";
const char txtFirefly[] PROGMEM = "IAW Firefly";
const char txtFiat500[] PROGMEM = "IAW 4AFB 500";

// --- Ford (módulos reais; ainda sem sinal na tabela) ---
const char txtDuratec20[] PROGMEM = "EEC-VI Duratec20";
const char txtDuratec23[] PROGMEM = "EEC-VI Duratec23";
const char txtTiVct[] PROGMEM     = "IAW 4CFR Sigma";
const char txtZetec[] PROGMEM     = "EEC-V Zetec";

// --- GM/Chevrolet ---
const char txtBoschM15[] PROGMEM   = "Bosch M1.5.5";
const char txtEconoflex[] PROGMEM  = "Multec H Econo";
const char txtFlexpower[] PROGMEM  = "ME7.9.9 Flexpwr";
const char txtMotronic15[] PROGMEM = "Motronic M1.5.x";
const char txtMultecVhc[] PROGMEM  = "Multec HSFI VHC";

// --- Honda (pendente: tipSin 12+1 não implementado) ---
const char txtD16w7[] PROGMEM      = "PGM-FI D16 pend.";
const char txtMotorL13a[] PROGMEM  = "PGM-FI L13A pend.";
const char txtMotorIvetc[] PROGMEM = "PGM-FI iVTEC pend.";
// --- Peugeot ---
const char txtEw10a[] PROGMEM     = "ME7.4.9 EW10A";
const char txt5Np1[] PROGMEM      = "IAW 5NP1/5NP2";
const char txtSagem2306[] PROGMEM = "Sagem SL96 306";
const char txtMe7416[] PROGMEM    = "Bosch ME7.4.4";
const char txtMe7216[] PROGMEM    = "Bosch ME7.2/7.3";

// --- Renault ---
const char txtF4rk4r[] PROGMEM      = "EMS F4R/K4M";
const char txtFenix3[] PROGMEM      = "Fenix 3/5";
const char txtk7m[] PROGMEM         = "Sirius K7M 1.6";
const char txt5nr1[] PROGMEM        = "IAW 5NR1/5NR2";
const char txtSagen3[] PROGMEM      = "Sagem Safir";
const char txtSirius32[] PROGMEM    = "Sirius 32";
const char txtSirius3134[] PROGMEM  = "Sirius 32/34";

// --- Diesel Leve: marcas ---
const char txtDlVw[] PROGMEM      = "VW";
const char txtDlGm[] PROGMEM      = "GM/Chevrolet";
const char txtDlFord[] PROGMEM    = "Ford";
const char txtDlFiat[] PROGMEM    = "Fiat";
const char txtDlPsa[] PROGMEM     = "Peugeot/Citroen";
const char txtDlMb[] PROGMEM      = "Mercedes-Benz";
const char txtDlNissan[] PROGMEM  = "Nissan";
const char txtDlToyota[] PROGMEM  = "Toyota";
const char txtDlTroller[] PROGMEM = "Troller";
const char txtDlIveco[] PROGMEM   = "Iveco";

// --- Diesel Leve: módulos (Master Diesel / MultiMEC) ---
const char txtEdc16c8[] PROGMEM   = "EDC16C8 Delivery";
const char txtEdc17Amarok[] PROGMEM = "EDC17 Amarok";
const char txtEdc16c9[] PROGMEM   = "EDC16C9 S10";
const char txtSid901[] PROGMEM    = "SID901 Ranger3.0";
const char txtSid208[] PROGMEM    = "SID208/209 Rang.";
const char txtEdc15Duc[] PROGMEM  = "EDC15 Ducato";
const char txtEdc16Duc[] PROGMEM  = "EDC16C39 Ducato";
const char txtEdc17Toro[] PROGMEM = "EDC17C69 Toro";
const char txtEdc15Psa[] PROGMEM  = "EDC15C7 Boxer";
const char txtEdc15Mb[] PROGMEM   = "EDC15C6 Sprinter";
const char txtEdc16Nis[] PROGMEM  = "EDC16 Frontier";
const char txtDensoHilux[] PROGMEM = "Denso Hilux CR";
const char txtSid901Tr[] PROGMEM  = "SID901 Troller";
const char txtEdc16Ive[] PROGMEM  = "EDC16 Daily";

// --- Diesel Pesado: marcas ---
const char txtDpMb[] PROGMEM      = "Mercedes-Benz";
const char txtDpScania[] PROGMEM  = "Scania";
const char txtDpIveco[] PROGMEM   = "Iveco";
const char txtDpVolvo[] PROGMEM   = "Volvo";
const char txtDpCummins[] PROGMEM = "Cummins";
const char txtDpMwm[] PROGMEM     = "MWM";
const char txtDpMan[] PROGMEM     = "MAN/VW Cam.";

// --- Diesel Pesado: módulos (Master Diesel / H-BOX / Chiptronic) ---
const char txtPldMr[] PROGMEM     = "PLD/MR Temic";
const char txtEmsS6[] PROGMEM     = "EMS S6/PDE";
const char txtMs62Sca[] PROGMEM   = "MS6.2 Bosch";
const char txtMs62Ive[] PROGMEM   = "EDC MS6.2";
const char txtMs63Ive[] PROGMEM   = "EDC MS6.3";
const char txtEdc7Fpt[] PROGMEM   = "EDC7 FPT";
const char txtVolvoD12[] PROGMEM  = "D12A/C/D";
const char txtVolvoD13[] PROGMEM  = "D13 EMS";
const char txtEdc7Isb4[] PROGMEM  = "EDC7 ISB 4cil";
const char txtEdc7Isb6[] PROGMEM  = "EDC7 ISB 6cil";
const char txtEdc7c14[] PROGMEM   = "EDC7C1 4cil";
const char txtEdc7c16[] PROGMEM   = "EDC7C1 6cil";
const char txtEdc7Uc31[] PROGMEM  = "EDC7 UC31 EuroV";
const char txtEdc7C32[] PROGMEM   = "EDC7 C32 MAN";

// --- Hall/Magnético e Tensão ---
const char txtSinalHall[] PROGMEM = "Sinal Hall";
const char txtMagnetico[] PROGMEM = "Sinal Magnetico";
const char txt12V[] PROGMEM      = "12 Volts";
const char txt5V[] PROGMEM       = "5 Volts";
const char txtRpmPot[] PROGMEM        = "RPM Potenciometro";
const char txtRpm800[] PROGMEM        = "RPM Fixo 800";
const char txtRpm2000[] PROGMEM       = "RPM Fixo 2000";
const char txtRpm4000[] PROGMEM       = "RPM Fixo 4000";
const char txtRpmSweep[] PROGMEM      = "RPM Varredura";
const char txtEditarFase[] PROGMEM    = "Editar Fase";
const char txtResetFase[] PROGMEM     = "Restaurar Fase";
const char txtInvFonica[] PROGMEM     = "Inverter Fonica";
const char txtInvFase[] PROGMEM       = "Inverter Fase";

// ====
// DEFINIÇÃO DOS MENUS (PROGMEM)
// ====

// --- Submenus de configuração ---
const MenuItem submenuHallMagnetico[] PROGMEM = {
  MENU_ITEM(NULL, txtSinalHall, &releHallMagnetico, 1),
  MENU_ITEM(NULL, txtMagnetico, &releHallMagnetico, 0),
  MENU_END
};

const MenuItem submenu5x12Volts[] PROGMEM = {
  MENU_ITEM(NULL, txt12V, &releHall5x12, 1),
  MENU_ITEM(NULL, txt5V,  &releHall5x12, 0),
  MENU_END
};

const MenuItem submenuAjustes[] PROGMEM = {
  MENU_ITEM(NULL, txtRpmPot,     &acaoModoRpm,         0),
  MENU_ITEM(NULL, txtRpm800,     &acaoModoRpm,         1),
  MENU_ITEM(NULL, txtRpm2000,    &acaoModoRpm,         2),
  MENU_ITEM(NULL, txtRpm4000,    &acaoModoRpm,         3),
  MENU_ITEM(NULL, txtRpmSweep,   &acaoModoRpm,         4),
  MENU_ITEM(NULL, txtEditarFase, &acaoAbrirEditorFase, 0),
  MENU_ITEM(NULL, txtResetFase,  &acaoRestaurarFase,   0),
  MENU_ITEM(NULL, txtInvFonica,  &acaoInverterFonica,  0),
  MENU_ITEM(NULL, txtInvFase,    &acaoInverterFase,    0),
  MENU_END
};

// --- Submenus por fabricante ---
// Índices = posição 1-based em sinais[] (Sinais.cpp). TFSI usa submenu Hall
// (valParametro=6 → AtivaMonitorAnalogico via releHallMagnetico).
const MenuItem submenuAudi[] PROGMEM = {
  MENU_ITEM(submenuHallMagnetico, txtAudiTfsi, NULL, 6),
  MENU_ITEM(NULL, txtAudiAtf, &AtivaMonitorAnalogico, 7),
  MENU_ITEM(NULL, txt1AvpAvb, &AtivaMonitorAnalogico, 8),
  MENU_ITEM(NULL, txt1AvsAvi, &AtivaMonitorAnalogico, 9),
  MENU_ITEM(NULL, txt4bvAvp,  &AtivaMonitorAnalogico, 10),
  MENU_ITEM(NULL, txt4Gv,     &AtivaMonitorAnalogico, 11),
  MENU_ITEM(NULL, txt4lv,     &AtivaMonitorAnalogico, 12),
  MENU_ITEM(NULL, txt5Wp4,    &AtivaMonitorAnalogico, 13),
  MENU_ITEM(NULL, txtMp9,     &AtivaMonitorAnalogico, 14),
  MENU_ITEM(NULL, txtMe7x,    &AtivaMonitorAnalogico, 15),
  MENU_ITEM(NULL, txtSimos,   &AtivaMonitorAnalogico, 16),
  MENU_END
};

const MenuItem submenuBmw[] PROGMEM = {
  MENU_ITEM(NULL, txt320i, &AtivaMonitorAnalogico, 17),
  MENU_ITEM(NULL, txtX1,   &AtivaMonitorAnalogico, 18),
  MENU_ITEM(NULL, txtM3,   &AtivaMonitorAnalogico, 19),
  MENU_END
};

const MenuItem submenuChrysler[] PROGMEM = {
  MENU_ITEM(NULL, txtChrys1, &AtivaMonitorAnalogico, 20),
  MENU_ITEM(NULL, txtChrys2, &AtivaMonitorAnalogico, 21),
  MENU_END
};

const MenuItem submenuCitroen[] PROGMEM = {
  MENU_ITEM(NULL, txtIaw8p,   &AtivaMonitorAnalogico, 22),
  MENU_ITEM(NULL, txtMe72,    &AtivaMonitorAnalogico, 23),
  MENU_ITEM(NULL, txtMe74,    &AtivaMonitorAnalogico, 24),
  MENU_ITEM(NULL, txtMa3,     &AtivaMonitorAnalogico, 25),
  MENU_ITEM(NULL, txtSagen,   &AtivaMonitorAnalogico, 26),
  MENU_ITEM(NULL, txt6pl1,    &AtivaMonitorAnalogico, 27),
  MENU_ITEM(NULL, txtMareli8, &AtivaMonitorAnalogico, 28),
  MENU_END
};

const MenuItem submenuFiat[] PROGMEM = {
  MENU_ITEM(NULL, txt1g7,     &AtivaMonitorAnalogico, 84),
  MENU_ITEM(NULL, txtEtorq,   &AtivaMonitorAnalogico, 29),
  MENU_ITEM(NULL, txtFire,    &AtivaMonitorAnalogico, 30),
  MENU_ITEM(NULL, txtEvo,     &AtivaMonitorAnalogico, 31),
  MENU_ITEM(NULL, txtMarea,   &AtivaMonitorAnalogico, 32),
  MENU_ITEM(NULL, txtFirefly, &AtivaMonitorAnalogico, 33),
  MENU_ITEM(NULL, txtFiat500, &AtivaMonitorAnalogico, 34),
  MENU_END
};

const MenuItem submenuFord[] PROGMEM = {
  MENU_ITEM(NULL, txtDuratec20, &AtivaMonitorAnalogico, 35),
  MENU_ITEM(NULL, txtDuratec23, &AtivaMonitorAnalogico, 36),
  MENU_ITEM(NULL, txtTiVct,     &AtivaMonitorAnalogico, 37),
  MENU_ITEM(NULL, txtZetec,     &AtivaMonitorAnalogico, 38),
  MENU_END
};

const MenuItem submenuGm[] PROGMEM = {
  MENU_ITEM(NULL, txtBoschM15,   &AtivaMonitorAnalogico, 39),
  MENU_ITEM(NULL, txtEconoflex,  &AtivaMonitorAnalogico, 40),
  MENU_ITEM(NULL, txtFlexpower,  &AtivaMonitorAnalogico, 41),
  MENU_ITEM(NULL, txtMotronic15, &AtivaMonitorAnalogico, 42),
  MENU_ITEM(NULL, txtMultecVhc,  &AtivaMonitorAnalogico, 43),
  MENU_END
};

const MenuItem submenuHonda[] PROGMEM = {
  // Pendente: PGM-FI L13A costuma usar 12+1 (não há tipSin em baseSinais[])
  MENU_ITEM(NULL, txtD16w7,      NULL, 0),
  MENU_ITEM(NULL, txtMotorL13a,  NULL, 0),
  MENU_ITEM(NULL, txtMotorIvetc, NULL, 0),
  MENU_END
};

const MenuItem submenuPeugeot[] PROGMEM = {
  MENU_ITEM(NULL, txtEw10a,     &AtivaMonitorAnalogico, 44),
  MENU_ITEM(NULL, txt5Np1,      &AtivaMonitorAnalogico, 45),
  MENU_ITEM(NULL, txtSagem2306, &AtivaMonitorAnalogico, 46),
  MENU_ITEM(NULL, txtMe7416,    &AtivaMonitorAnalogico, 47),
  MENU_ITEM(NULL, txtMe7216,    &AtivaMonitorAnalogico, 48),
  MENU_END
};

const MenuItem submenuRenault[] PROGMEM = {
  MENU_ITEM(NULL, txtF4rk4r,      &AtivaMonitorAnalogico, 49),
  MENU_ITEM(NULL, txtFenix3,      &AtivaMonitorAnalogico, 50),
  MENU_ITEM(NULL, txtk7m,         &AtivaMonitorAnalogico, 51),
  MENU_ITEM(NULL, txt5nr1,        &AtivaMonitorAnalogico, 52),
  MENU_ITEM(NULL, txtSagen3,      &AtivaMonitorAnalogico, 53),
  MENU_ITEM(NULL, txtSirius32,    &AtivaMonitorAnalogico, 54),
  MENU_ITEM(NULL, txtSirius3134,  &AtivaMonitorAnalogico, 55),
  MENU_END
};

// --- Diesel Leve (índices 56-69 em sinais[]) ---
const MenuItem submenuDlVw[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc16c8,     &AtivaMonitorAnalogico, 56),
  MENU_ITEM(NULL, txtEdc17Amarok, &AtivaMonitorAnalogico, 57),
  MENU_END
};

const MenuItem submenuDlGm[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc16c9, &AtivaMonitorAnalogico, 58),
  MENU_END
};

const MenuItem submenuDlFord[] PROGMEM = {
  MENU_ITEM(NULL, txtSid901, &AtivaMonitorAnalogico, 59),
  MENU_ITEM(NULL, txtSid208, &AtivaMonitorAnalogico, 60),
  MENU_END
};

const MenuItem submenuDlFiat[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc15Duc,  &AtivaMonitorAnalogico, 61),
  MENU_ITEM(NULL, txtEdc16Duc,  &AtivaMonitorAnalogico, 62),
  MENU_ITEM(NULL, txtEdc17Toro, &AtivaMonitorAnalogico, 63),
  MENU_END
};

const MenuItem submenuDlPsa[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc15Psa, &AtivaMonitorAnalogico, 64),
  MENU_END
};

const MenuItem submenuDlMb[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc15Mb, &AtivaMonitorAnalogico, 65),
  MENU_END
};

const MenuItem submenuDlNissan[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc16Nis, &AtivaMonitorAnalogico, 66),
  MENU_END
};

const MenuItem submenuDlToyota[] PROGMEM = {
  MENU_ITEM(NULL, txtDensoHilux, &AtivaMonitorAnalogico, 67),
  MENU_END
};

const MenuItem submenuDlTroller[] PROGMEM = {
  MENU_ITEM(NULL, txtSid901Tr, &AtivaMonitorAnalogico, 68),
  MENU_END
};

const MenuItem submenuDlIveco[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc16Ive, &AtivaMonitorAnalogico, 69),
  MENU_END
};

const MenuItem submenuDieselLeve[] PROGMEM = {
  MENU_ITEM(submenuDlVw,     txtDlVw,     NULL, 0),
  MENU_ITEM(submenuDlGm,     txtDlGm,     NULL, 0),
  MENU_ITEM(submenuDlFord,   txtDlFord,   NULL, 0),
  MENU_ITEM(submenuDlFiat,   txtDlFiat,   NULL, 0),
  MENU_ITEM(submenuDlPsa,    txtDlPsa,    NULL, 0),
  MENU_ITEM(submenuDlMb,     txtDlMb,     NULL, 0),
  MENU_ITEM(submenuDlNissan, txtDlNissan, NULL, 0),
  MENU_ITEM(submenuDlToyota, txtDlToyota, NULL, 0),
  MENU_ITEM(submenuDlTroller,txtDlTroller,NULL, 0),
  MENU_ITEM(submenuDlIveco,  txtDlIveco,  NULL, 0),
  MENU_END
};

// --- Diesel Pesado (índices 70-83 em sinais[]) ---
const MenuItem submenuDpMb[] PROGMEM = {
  MENU_ITEM(NULL, txtPldMr, &AtivaMonitorAnalogico, 70),
  MENU_END
};

const MenuItem submenuDpScania[] PROGMEM = {
  MENU_ITEM(NULL, txtEmsS6,   &AtivaMonitorAnalogico, 71),
  MENU_ITEM(NULL, txtMs62Sca, &AtivaMonitorAnalogico, 72),
  MENU_END
};

const MenuItem submenuDpIveco[] PROGMEM = {
  MENU_ITEM(NULL, txtMs62Ive, &AtivaMonitorAnalogico, 73),
  MENU_ITEM(NULL, txtMs63Ive, &AtivaMonitorAnalogico, 74),
  MENU_ITEM(NULL, txtEdc7Fpt, &AtivaMonitorAnalogico, 75),
  MENU_END
};

const MenuItem submenuDpVolvo[] PROGMEM = {
  MENU_ITEM(NULL, txtVolvoD12, &AtivaMonitorAnalogico, 76),
  MENU_ITEM(NULL, txtVolvoD13, &AtivaMonitorAnalogico, 77),
  MENU_END
};

const MenuItem submenuDpCummins[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc7Isb4, &AtivaMonitorAnalogico, 78),
  MENU_ITEM(NULL, txtEdc7Isb6, &AtivaMonitorAnalogico, 79),
  MENU_END
};

const MenuItem submenuDpMwm[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc7c14,  &AtivaMonitorAnalogico, 80),
  MENU_ITEM(NULL, txtEdc7c16,  &AtivaMonitorAnalogico, 81),
  MENU_ITEM(NULL, txtEdc7Uc31, &AtivaMonitorAnalogico, 82),
  MENU_END
};

const MenuItem submenuDpMan[] PROGMEM = {
  MENU_ITEM(NULL, txtEdc7C32, &AtivaMonitorAnalogico, 83),
  MENU_END
};

const MenuItem submenuDieselPesado[] PROGMEM = {
  MENU_ITEM(submenuDpMb,     txtDpMb,     NULL, 0),
  MENU_ITEM(submenuDpScania, txtDpScania, NULL, 0),
  MENU_ITEM(submenuDpIveco,  txtDpIveco,  NULL, 0),
  MENU_ITEM(submenuDpVolvo,  txtDpVolvo,  NULL, 0),
  MENU_ITEM(submenuDpCummins,txtDpCummins,NULL, 0),
  MENU_ITEM(submenuDpMwm,    txtDpMwm,    NULL, 0),
  MENU_ITEM(submenuDpMan,    txtDpMan,    NULL, 0),
  MENU_END
};

// --- Menu de Carros (ordem: volume BR + ECU TEC/ECUPROG) ---
const MenuItem submenuCarros[] PROGMEM = {
  MENU_ITEM(submenuAudi,     txtAudiSeatVw, NULL, 0),
  MENU_ITEM(submenuFiat,     txtFiat,       NULL, 0),
  MENU_ITEM(submenuGm,       txtGM,         NULL, 0),
  MENU_ITEM(submenuFord,     txtFord,       NULL, 0),
  MENU_ITEM(submenuRenault,  txtRenault,    NULL, 0),
  MENU_ITEM(submenuPeugeot,  txtPeugeot,    NULL, 0),
  MENU_ITEM(submenuCitroen,  txtCitroen,    NULL, 0),
  MENU_ITEM(submenuHonda,    txtHonda,      NULL, 0),
  MENU_ITEM(NULL,            txtToyota,     NULL, 0),
  MENU_ITEM(submenuBmw,      txtBmw,        NULL, 0),
  MENU_ITEM(submenuChrysler, txtCrysler,    NULL, 0),
  MENU_END
};

// --- Menu de Sinais Básicos ---
const MenuItem subMenuSinaisBasicos[] PROGMEM = {
  MENU_ITEM(submenu5x12Volts,     txtHall, NULL, 1),
  MENU_ITEM(submenuHallMagnetico, txt36x1, NULL, 2),
  MENU_ITEM(submenuHallMagnetico, txt36x2, NULL, 3),
  MENU_ITEM(submenuHallMagnetico, txt44x4, NULL, 4),
  MENU_ITEM(submenuHallMagnetico, txt60x2, NULL, 5),
  MENU_END
};

// --- Menu Principal ---
const MenuItem menuPrincipal[] PROGMEM = {
  MENU_ITEM(submenuCarros,         txtCarros,        NULL, 0),
  MENU_ITEM(submenuDieselLeve,     txtDieselLeve,    NULL, 0),
  MENU_ITEM(submenuDieselPesado,   txtDieselPesado,  NULL, 0),
  MENU_ITEM(subMenuSinaisBasicos,  txtSinaisBasicos, NULL, 0),
  MENU_ITEM(submenuAjustes,         txtAjustes,       NULL, 0),
  MENU_END
};

// ====
// TABELA DE TÍTULOS DOS MENUS
// ====
const MenuTitulo titulosMenus[] PROGMEM = {
  { menuPrincipal,        tituloPrincipal },
  { subMenuSinaisBasicos, tituloSinaisBasicos },
  { submenuCarros,        tituloCarros },
  { submenuAudi,          tituloAudi },
  { submenuBmw,           tituloBmw },
  { submenuFiat,          tituloFiat },
  { submenuChrysler,      tituloChrysler },
  { submenuCitroen,       tituloCitroen },
  { submenuFord,          tituloFord },
  { submenuGm,            tituloGm },
  { submenuHonda,         tituloHonda },
  { submenuPeugeot,       tituloPeugeot },
  { submenuRenault,       tituloRenault },
  { submenuDieselLeve,    tituloDieselLeve },
  { submenuDlVw,          tituloDlVw },
  { submenuDlGm,          tituloDlGm },
  { submenuDlFord,        tituloDlFord },
  { submenuDlFiat,        tituloDlFiat },
  { submenuDlPsa,         tituloDlPsa },
  { submenuDlMb,          tituloDlMb },
  { submenuDlNissan,      tituloDlNissan },
  { submenuDlToyota,      tituloDlToyota },
  { submenuDlTroller,     tituloDlTroller },
  { submenuDlIveco,       tituloDlIveco },
  { submenuDieselPesado,  tituloDieselPesado },
  { submenuDpMb,          tituloDpMb },
  { submenuDpScania,      tituloDpScania },
  { submenuDpIveco,       tituloDpIveco },
  { submenuDpVolvo,       tituloDpVolvo },
  { submenuDpCummins,     tituloDpCummins },
  { submenuDpMwm,         tituloDpMwm },
  { submenuDpMan,         tituloDpMan },
  { submenuHallMagnetico, tituloHallMagnetico },
  { submenu5x12Volts,     titulo5x12Volts },
  { submenuAjustes,       tituloAjustes }
};

const uint8_t titulosMenusCount = sizeof(titulosMenus) / sizeof(MenuTitulo); // Calcula a quantidade de títulos no arquivo onde o array tem tamanho completo

// ====
// FUNÇÕES UTILITÁRIAS
// ====

uint8_t contarItens(const MenuItem *menu) {
  uint8_t count = 0;
  while (pgm_read_ptr(&menu[count].texto)) count++;
  return count;
}

const char *readMenuItemTexto(const MenuItem *itemPtr, char *buffer, size_t bufSize) {
  const char *textoPtr = (const char *)pgm_read_ptr(&itemPtr->texto);
  if (!textoPtr) {
    buffer[0] = '\0';
    return buffer;
  }
  strncpy_P(buffer, textoPtr, bufSize);
  buffer[bufSize - 1] = '\0';
  return buffer;
}

MenuItem readMenuItem(const MenuItem *itemPtr) {
  MenuItem temp;
  temp.submenu   = (const MenuItem *)pgm_read_ptr(&itemPtr->submenu);
  temp.texto     = (const char *)pgm_read_ptr(&itemPtr->texto);
  temp.acao      = (void (*)(uint8_t))pgm_read_ptr(&itemPtr->acao);
  temp.parametro = pgm_read_byte(&itemPtr->parametro);
  return temp;
}