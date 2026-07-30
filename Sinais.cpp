/**
 * ============================================================================
 * Sinais.cpp - Implementação do Módulo de Sinais
 * ============================================================================
 * 
 * COMO ADICIONAR UM NOVO VEÍCULO:
 * --------------------------------
 * 1. Na tabela 'sinais[]' abaixo, adicione uma nova linha antes do último item
 * 2. Preencha os campos:
 *    - tipSin: tipo de roda fônica (1=Hall, 2=36-1, 3=36-2, 4=44-4, 5=60-2, 6=60-1+1)
 *    - tipFse: número de dentes de fase (0=sem fase, 1..4)
 *    - dnt1/lrd1 até dnt4/lrd4: posição e largura dos dentes de fase
 * 3. Atualize NUM_SINAIS em Sinais.h
 * 4. Adicione o texto PROGMEM e a entrada de menu em MenuTextos.h e MenuData.cpp
 * 
 * Exemplo:
 *   { 5, 3, 30, 70, 90, 130, 150, 210, 0, 0 },  // Novo Veículo X
 * ============================================================================
 */

#include "Sinais.h"
#include <avr/interrupt.h>

// ============================================================================
// DEFINIÇÃO DAS VARIÁVEIS GLOBAIS
// ============================================================================

volatile int _pulso = 0, _falha = 0, _falha1 = 0;
volatile bool _hall = false, _fonica36 = false, _fonica44 = false;
volatile bool _fonica62 = false, _fonica = false, _fonica1 = false;
volatile bool _simulando = false;
volatile bool sinalSelecionado = false;
volatile uint8_t sinalAtual = 0;
volatile uint8_t tipoSinalAtual = 1;

volatile int _faseOK = 0;
volatile int _pI1D = 0, _l1D = 0, _pI2D = 0, _l2D = 0;
volatile int _pI3D = 0, _l3D = 0, _pI4D = 0, _l4D = 0, _fSF = 0;

volatile int _cP = 0;
volatile int _cF = 0;
volatile uint8_t valParametro = 0;

// ============================================================================
// TABELA DE SINAIS POR VEÍCULO (PROGMEM)
// ============================================================================
// Formato: { tipSin, tipFse, dnt1, lrd1, dnt2, lrd2, dnt3, lrd3, dnt4, lrd4 }
//
// tipSin: 1=Hall contínuo, 2=36-1, 3=36-2, 4=44-4, 5=60-2, 6=60-1+1

const SinalConfig sinais[] PROGMEM = {
  // === Sinais Básicos (índices 1-5) ===
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               //  1: Hall contínuo
  { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               //  2: 36-1
  { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               //  3: 36-2
  { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               //  4: 44-4
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               //  5: 60-2

  // === VW/Audi/Seat — módulos IAW / ME / MP9 / Simos (índices 6-16) ===
  { 5, 4, 30, 46, 56, 104, 114, 164, 214, 226 },  //  6: ME TFSI 1.8/2.0
  { 5, 4, 26, 42, 58, 102, 120, 162, 206, 222 },  //  7: ATF 2.0 8V
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               //  8: IAW 1AVP/1AVB
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               //  9: IAW 1AVS/1AVI
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 10: IAW 4BV/4AVP
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 11: IAW 4GV / ME
  { 6, 3, 28, 68, 86, 126, 144, 204, 0, 0 },      // 12: IAW 4LV/4SV/4MV
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 13: IAW 5WP4
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 14: Bosch MP9.0
  { 6, 4, 26, 38, 56, 100, 115, 158, 208, 220 },  // 15: Bosch ME7.5.x
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 16: Siemens Simos

  // === BMW (índices 17-19) ===
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 17: MSD80 320i
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 18: MSD80 X1
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 19: MSS60 M3

  // === Chrysler (índices 20-21) ===
  { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 20: SBEC 1.8/2.0
  { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 21: SBEC 2.2/2.5

  // === Citroën / PSA (índices 22-28) ===
  { 5, 4, 8, 30, 48, 90, 108, 150, 168, 186 },    // 22: IAW 8P/1AP/48P
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 23: Bosch ME7.2/7.3
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 24: Bosch ME7.4.4 (60-2)
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 25: MA3.0/3.1/5.1
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 26: Sagem SL96/2000
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 27: IAW 6LP1/6LPB
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 28: IAW 8P20

  // === Fiat (índices 29-34) ===
  { 5, 1, 72, 190, 0, 0, 0, 0, 0, 0 },            // 29: ME7.9.9 Etorq
  { 5, 3, 32, 72, 92, 136, 154, 212, 0, 0 },      // 30: IAW 4AFB Fire
  { 5, 3, 30, 74, 92, 130, 148, 206, 0, 0 },      // 31: IAW 7GF Evo
  { 5, 1, 12, 240, 0, 0, 0, 0, 0, 0 },            // 32: IAW 59FB Marea
  { 5, 3, 26, 68, 86, 128, 144, 202, 0, 0 },      // 33: IAW Firefly
  // 34: IAW 4AFB Fiat 500 (família Fire)
  { 5, 3, 32, 72, 92, 136, 154, 212, 0, 0 },

  // === Ford (índices 35-38) — tipSin conhecido; fase tipFse=0 (nível A) ===
  { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 35: EEC-VI Duratec 2.0 (36-1)
  { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 36: EEC-VI Duratec 2.3 (36-1)
  { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 37: IAW 4CFR Sigma (36-1)
  { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 38: EEC-V Zetec (36-1)

  // === GM/Chevrolet (índices 39-43) — 60-2; fase tipFse=0 ===
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 39: Bosch M1.5.5
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 40: Multec H Econo.Flex
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 41: ME7.9.9 Flexpower
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 42: Motronic M1.5.x
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 43: Multec HSFI VHC

  // === Peugeot (índices 44-48) — espelha família PSA / 60-2 ===
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 44: ME7.4.9 EW10A
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 45: IAW 5NP1/5NP2
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 46: Sagem SL96 306
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 47: Bosch ME7.4.4
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 48: Bosch ME7.2/7.3

  // === Renault (índices 49-55) — 60-2; fase tipFse=0 ===
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 49: EMS F4R/K4M
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 50: Fenix 3/5
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 51: Sirius K7M 1.6
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 52: IAW 5NR1/5NR2
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 53: Sagem Safir
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 54: Sirius 32
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 55: Sirius 32/34

  // === Diesel Leve (índices 56-69) — 60-2; fase tipFse=0 (nível A) ===
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 56: VW EDC16 C8 Delivery
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 57: VW EDC17 Amarok
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 58: GM EDC16C9 S10/Blazer
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 59: Ford SID901 Ranger 3.0
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 60: Ford SID208/209 Ranger
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 61: Fiat EDC15 Ducato
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 62: Fiat EDC16C39 Ducato
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 63: Fiat EDC17C69 Toro
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 64: PSA EDC15C7 Boxer/Jumper
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 65: MB EDC15C6 Sprinter
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 66: Nissan EDC16 Frontier
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 67: Toyota Denso Hilux
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 68: Troller SID901
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 69: Iveco Daily EDC16

  // === Diesel Pesado (índices 70-83) — 60-2; fase tipFse=0 (nível A) ===
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 70: MB Temic PLD/MR
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 71: Scania EMS S6/PDE
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 72: Scania Bosch MS6.2
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 73: Iveco EDC MS6.2
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 74: Iveco EDC MS6.3
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 75: Iveco EDC7 FPT
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 76: Volvo D12A/C/D
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 77: Volvo D13
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 78: Cummins EDC7 ISB 4cil
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 79: Cummins EDC7 ISB 6cil
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 80: MWM EDC7C1 4cil
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 81: MWM EDC7C1 6cil
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 82: MWM EDC7 UC31 EuroV
  { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 83: MAN/VW EDC7 C32

  // === Fiat suplementar (índice 84) ===
  // IAW 1G7/G7 — 60-2; fase 1 dente (Palio/Uno/Siena Marelli)
  { 5, 1, 12, 240, 0, 0, 0, 0, 0, 0 }             // 84: IAW 1G7/G7
};

// ============================================================================
// TABELA BASE - Tipos de Roda Fônica (PROGMEM)
// ============================================================================
// Formato: { tipo, pulso, falha, falha1, hall, f36, f44, f62, fonica, fonica1, Ccf }

const BaseConfig baseSinais[] PROGMEM = {
  { 1,   0,   0,   0, true,  false, false, false, false, false,   0 },  // Hall contínuo
  { 2,  72,  74,   0, false, true,  false, false, true,  false, 124 },  // 36-1
  { 3,  72,  76,   0, false, true,  false, false, true,  false, 124 },  // 36-2
  { 4,  88,  96,   0, false, false, true,  false, true,  false, 176 },  // 44-4
  { 5, 116, 120,   0, false, false, false, true,  true,  false, 240 },  // 60-2
  { 6, 116, 118, 120, false, false, false, true,  true,  true,  240 }   // 60-1+1
};

// ============================================================================
// IMPLEMENTAÇÃO DAS FUNÇÕES
// ============================================================================

void ConfigSinFase(int tipSin, int tipFse,
                   int dnt1, int lrd1, int dnt2, int lrd2,
                   int dnt3, int lrd3, int dnt4, int lrd4) {
  // --- Carrega configuração base do tipo de roda fônica ---
  BaseConfig base;
  memcpy_P(&base, &baseSinais[tipSin - 1], sizeof(BaseConfig));

  uint8_t sreg = SREG;
  cli();
  tipoSinalAtual = (uint8_t)tipSin;
  _pulso    = base.pulso;
  _falha    = base.falha;
  _falha1   = base.falha1;
  _hall     = base.hall;
  _fonica36 = base.fonica36;
  _fonica44 = base.fonica44;
  _fonica62 = base.fonica62;
  _fonica   = base.fonica;
  _fonica1  = base.fonica1;
  _fSF      = base.Ccf;

  // --- Configura dentes de fase conforme quantidade ---
  _faseOK = tipFse;
  _cF = 0;
  _pI1D = _l1D = _pI2D = _l2D = 0;
  _pI3D = _l3D = _pI4D = _l4D = 0;
  if (tipFse >= 1) { _pI1D = dnt1; _l1D = lrd1; }
  if (tipFse >= 2) { _pI2D = dnt2; _l2D = lrd2; }
  if (tipFse >= 3) { _pI3D = dnt3; _l3D = lrd3; }
  if (tipFse >= 4) { _pI4D = dnt4; _l4D = lrd4; }
  SREG = sreg;
}

void SelecionaSin(uint8_t valor) {
  Serial.print(F("Selecionando sinal: "));
  Serial.println(valor);

  if (valor == 0 || valor > NUM_SINAIS) return;

  // Lê configuração do sinal selecionado da PROGMEM
  SinalConfig cfg;
  memcpy_P(&cfg, &sinais[valor - 1], sizeof(SinalConfig));
  sinalAtual = valor;
  sinalSelecionado = true;

  // Aplica a configuração
  ConfigSinFase(cfg.tipSin, cfg.tipFse,
                cfg.dnt1, cfg.lrd1,
                cfg.dnt2, cfg.lrd2,
                cfg.dnt3, cfg.lrd3,
                cfg.dnt4, cfg.lrd4);
}

void printConfig() {
  Serial.println(F("=== Configuracao Atual ==="));
  Serial.print(F("Pulso: "));     Serial.println(_pulso);
  Serial.print(F("Falha: "));     Serial.println(_falha);
  Serial.print(F("Falha1: "));    Serial.println(_falha1);
  Serial.print(F("Hall: "));      Serial.println(_hall ? "Sim" : "Nao");
  Serial.print(F("Fonica36: "));  Serial.println(_fonica36 ? "Sim" : "Nao");
  Serial.print(F("Fonica44: "));  Serial.println(_fonica44 ? "Sim" : "Nao");
  Serial.print(F("Fonica62: "));  Serial.println(_fonica62 ? "Sim" : "Nao");
  Serial.print(F("Fonica: "));    Serial.println(_fonica ? "Sim" : "Nao");
  Serial.print(F("Fonica1: "));   Serial.println(_fonica1 ? "Sim" : "Nao");
  Serial.print(F("Fase OK: "));   Serial.println(_faseOK);
  Serial.print(F("Fsf: "));       Serial.println(_fSF);

  if (_faseOK >= 1) {
    Serial.print(F("Dente1: ")); Serial.print(_pI1D);
    Serial.print(F(" Largura: ")); Serial.println(_l1D);
  }
  if (_faseOK >= 2) {
    Serial.print(F("Dente2: ")); Serial.print(_pI2D);
    Serial.print(F(" Largura: ")); Serial.println(_l2D);
  }
  if (_faseOK >= 3) {
    Serial.print(F("Dente3: ")); Serial.print(_pI3D);
    Serial.print(F(" Largura: ")); Serial.println(_l3D);
  }
  if (_faseOK >= 4) {
    Serial.print(F("Dente4: ")); Serial.print(_pI4D);
    Serial.print(F(" Largura: ")); Serial.println(_l4D);
  }
  Serial.println(F("===="));
}
