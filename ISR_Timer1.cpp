/**
 * ============================================================================
 * ISR_Timer1.cpp — Vetor de interrupção do Timer1 Compare Match A
 * ============================================================================
 *
 * Este arquivo existe em separado para garantir UMA única definição de
 * ISR(TIMER1_COMPA_vect). Se a ISR estivesse em um .h incluído por vários
 * .cpp, o linker reportaria "multiple definition".
 *
 * Fluxo a cada compare match (OCR1A):
 *   TIMER1_COMPA_vect → InterruptTimer1() → PORTA bits 4 e 5 (pinos 26 e 27)
 *
 * A ISR deve permanecer curta (~2–5 µs): apenas incrementa contadores e
 * altera PORTA. Leituras analógicas e LCD ficam no loop() / Timer4.
 * ============================================================================
 */

#include <avr/interrupt.h>
#include "Sensores.h"

ISR(TIMER1_COMPA_vect) {
  InterruptTimer1();
}
