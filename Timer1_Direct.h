/**
 * ============================================================================
 * Timer1_Direct.h — API do controle direto do Timer1 (ATmega2560)
 * ============================================================================
 *
 * O Timer1 gera interrupções periódicas que acionam InterruptTimer1() (ver
 * ISR_Timer1.cpp e Sensores.cpp). Cada interrupção avança um passo do padrão
 * de roda fônica (pino 26 / PA4) e do sinal de fase (pino 27 / PA5).
 *
 * Documentação detalhada: ARQUITETURA.md → seção "Timer1".
 *
 * Por que não usar a biblioteca TimerOne?
 *   - TimerOne fixa prescaler e limita o período máximo (~65 ms com prescaler 64).
 *   - Este módulo configura CTC + prescaler 64 de forma explícita e compartilha
 *     a mesma fórmula de OCR usada no loop (calcPeriodo → Timer1_SetPeriod).
 *
 * Arquivos relacionados:
 *   Timer1_Direct.cpp  — escrita nos registradores TCCR1*, OCR1A, TIMSK1
 *   ISR_Timer1.cpp     — vetor TIMER1_COMPA_vect (definição única da ISR)
 *   Sensores.cpp       — InterruptTimer1() (lógica do sinal)
 * ============================================================================
 */

#ifndef TIMER1_DIRECT_H
#define TIMER1_DIRECT_H

#include <Arduino.h>

/** Configura modo CTC e interrupção Compare A; timer fica PARADO até Timer1_Start(). */
void Timer1_Initialize();

/**
 * Ajusta o período entre interrupções.
 * @param periodo_us Período desejado em microssegundos (tipicamente 50–10.000 µs)
 */
void Timer1_SetPeriod(unsigned long periodo_us);

/** Zera TCCR1B — para contagem e interrupções por compare. */
void Timer1_Stop();

/** Reaplica prescaler 64 e modo CTC (após Timer1_Stop). */
void Timer1_Start();

#endif // TIMER1_DIRECT_H
