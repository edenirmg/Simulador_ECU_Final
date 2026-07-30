/**
 * ============================================================================
 * Sinais.h - Declarações do Módulo de Sinais
 * ============================================================================
 * 
 * Módulo responsável pela configuração e geração dos sinais de
 * roda fônica e fase. Contém:
 *   - Tabelas de configuração de sinais por veículo (PROGMEM)
 *   - Tabelas de configuração base por tipo de roda fônica (PROGMEM)
 *   - Funções de seleção e configuração de sinais
 *   - Variáveis globais de estado do sinal ativo
 * 
 * Para adicionar um novo veículo, veja instruções em Sinais.cpp
 * ============================================================================
 */

#ifndef SINAIS_H
#define SINAIS_H

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "Tipos.h"

// ============================================================================
// TABELAS DE CONFIGURAÇÃO (PROGMEM)
// ============================================================================

/** Tabela de sinais - um registro por veículo/ECU */
extern const SinalConfig sinais[] PROGMEM;

/** Tabela base - um registro por tipo de roda fônica */
extern const BaseConfig baseSinais[] PROGMEM;

/** Número total de sinais disponíveis na tabela */
#define NUM_SINAIS  84

// ============================================================================
// VARIÁVEIS GLOBAIS DE ESTADO DO SINAL
// ============================================================================

// Parâmetros do sinal ativo (copiados da tabela ao selecionar)
// Declarados volatile porque são LIDOS na ISR InterruptTimer1()
// e ESCRITOS em ConfigSinFase() / SelecionaSin() (chamados do loop).
extern volatile int _pulso, _falha, _falha1;
extern volatile bool _hall, _fonica36, _fonica44, _fonica62, _fonica, _fonica1;

// Flags de estado
extern volatile bool _simulando;        // true quando simulação está ativa
extern volatile bool sinalSelecionado;  // true após seleção de sinal válido
extern volatile uint8_t sinalAtual;      // Índice 1-based em sinais[]
extern volatile uint8_t tipoSinalAtual;  // Tipo base 1..6

// Estado da fase (lidos na ISR Timer1, escritos em ConfigSinFase)
extern volatile int _faseOK;
extern volatile int _pI1D, _l1D, _pI2D, _l2D, _pI3D, _l3D, _pI4D, _l4D, _fSF;

// Contadores de interrupção (volatile pois são modificados em ISR)
extern volatile int _cP;   // Contador de pulsos Hall/Fônica
extern volatile int _cF;   // Contador de fases

// Parâmetro do menu (valor passado pelo sistema de navegação)
extern volatile uint8_t valParametro;

// ============================================================================
// FUNÇÕES PÚBLICAS
// ============================================================================

/**
 * Configura o sinal e a fase a partir dos parâmetros fornecidos.
 * 
 * @param tipSin Tipo de sinal base (1..6, índice na tabela baseSinais)
 * @param tipFse Número de fases/dentes de referência (0..4)
 * @param dnt1..lrd4 Posições e larguras dos dentes de fase
 */
void ConfigSinFase(int tipSin, int tipFse,
                   int dnt1, int lrd1, int dnt2, int lrd2,
                   int dnt3, int lrd3, int dnt4, int lrd4);

/**
 * Seleciona um sinal da tabela pelo índice (1-based).
 * Carrega a configuração completa do veículo nas variáveis globais.
 * 
 * @param valor Índice do sinal (1..NUM_SINAIS)
 */
void SelecionaSin(uint8_t valor);

/**
 * Imprime a configuração atual no Serial Monitor (para debug).
 */
void printConfig();

#endif // SINAIS_H
