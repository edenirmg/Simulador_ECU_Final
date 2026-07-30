/**
 * ============================================================================
 * Timer1_Direct.cpp — Registradores do Timer1 no ATmega2560
 * ============================================================================
 *
 * Modo: CTC (Clear Timer on Compare Match), TOP = OCR1A
 * Clock: F_CPU = 16 MHz, prescaler = 64 → tick = 4 µs
 *
 * Período da interrupção:
 *   T_isr [µs] = 4 × (OCR1A + 1)
 *   OCR1A      = (periodo_us / 4) - 1   (ver Timer1_SetPeriod)
 *
 * Faixa prática com OCR1A de 16 bits (0–65535):
 *   Mínimo ≈ 4 µs   |  Máximo ≈ 262 ms por interrupção
 *
 * No simulador, o loop limita periodo_us entre TIMER1_MIN_US e TIMER1_MAX_US
 * (Config.h) antes de chamar Timer1_SetPeriod.
 * ============================================================================
 */

#include "Timer1_Direct.h"
#include "Config.h"
#include <avr/interrupt.h>

void Timer1_Initialize() {
  cli();

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // Modo CTC (WGM12) — relógio PARADO até Timer1_Start() (sem CS11/CS10)
  TCCR1B = (1 << WGM12);

  // Período seguro antes de qualquer Start (evita OCR1A=0 → ISR a cada 4 µs)
  Timer1_SetPeriod(TIMER1_PERIODO_US);

  TIMSK1 |= (1 << OCIE1A);

  sei();
}

void Timer1_SetPeriod(unsigned long periodo_us) {
  unsigned long ocr_value = (periodo_us / 4);
  if (ocr_value > 0) {
    ocr_value -= 1;
  }
  if (ocr_value > 65535) {
    ocr_value = 65535;
  }

  // Atualização atômica: evita OCR/TCNT inconsistentes e glitch ("corte")
  // quando o RPM sobe e o contador já passou do novo TOP.
  uint8_t sreg = SREG;
  cli();
  OCR1A = (uint16_t)ocr_value;
  if (TCNT1 >= OCR1A) {
    TCNT1 = 0;
  }
  SREG = sreg;
}

void Timer1_Stop() {
  TCCR1B = 0;
}

void Timer1_Start() {
  // Atribuição (=) garante modo CTC + prescaler 64 após Timer1_Stop() zerar TCCR1B
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
}
