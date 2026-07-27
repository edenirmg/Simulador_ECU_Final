/**
 * ============================================================================
 * Reles.cpp - Implementação do Controle dos Relés
 * ============================================================================
 */

#include "Reles.h"
#include "Config.h"

// Declaração externa da função do monitor (evita dependência circular)
extern void AtivaMonitorAnalogico(uint8_t param);

void inicializarReles() {
  pinMode(RELE_HALL_MAGNETICO, OUTPUT);
  digitalWrite(RELE_HALL_MAGNETICO, LOW);

  pinMode(RELE_5_12VOLTS, OUTPUT);
  digitalWrite(RELE_5_12VOLTS, LOW);

  pinMode(RELE_FASE_HALL_MAGNETICO, OUTPUT);
  digitalWrite(RELE_FASE_HALL_MAGNETICO, LOW);
}

void releOff() {
  digitalWrite(RELE_HALL_MAGNETICO, LOW);
  digitalWrite(RELE_5_12VOLTS, LOW);
  digitalWrite(RELE_FASE_HALL_MAGNETICO, LOW);
}

void releHallMagnetico(uint8_t valor) {
  if (valor == 1)
    digitalWrite(RELE_HALL_MAGNETICO, HIGH);
  else
    digitalWrite(RELE_HALL_MAGNETICO, LOW);

  // Após selecionar tipo de sinal, ativa o monitor analógico
  AtivaMonitorAnalogico(1);
}

void releHall5x12(uint8_t valor) {
  if (valor == 1)
    digitalWrite(RELE_5_12VOLTS, LOW);
  else
    digitalWrite(RELE_5_12VOLTS, HIGH);

  releHallMagnetico(1);
}
