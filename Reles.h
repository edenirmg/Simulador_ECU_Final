/**
 * ============================================================================
 * Reles.h - Módulo de Controle dos Relés
 * ============================================================================
 * 
 * Controla os relés de saída do simulador:
 *   - Relé Hall/Magnético: seleciona tipo de sinal de saída
 *   - Relé 5V/12V: seleciona tensão de saída Hall
 *   - Relé Fase Hall/Magnético: seleciona tipo de fase
 * ============================================================================
 */

#ifndef RELES_H
#define RELES_H

#include <Arduino.h>

/**
 * Inicializa os pinos dos relés como OUTPUT e desliga todos.
 * Deve ser chamada no setup().
 */
void inicializarReles();

/**
 * Desliga todos os relés (estado seguro).
 */
void releOff();

/**
 * Controla o relé de seleção Hall/Magnético.
 * @param valor 1 = Hall (HIGH), 0 = Magnético (LOW)
 */
void releHallMagnetico(uint8_t valor);

/**
 * Controla o relé de tensão Hall (5V ou 12V).
 * @param valor 1 = 12V (LOW no relé), 0 = 5V (HIGH no relé)
 */
void releHall5x12(uint8_t valor);

#endif // RELES_H
