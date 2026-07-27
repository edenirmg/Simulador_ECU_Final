/**
 * ============================================================================
 * Config.h — Configurações Centralizadas do Simulador de ECU v2.6
 * ============================================================================
 *
 * Este arquivo concentra TODAS as constantes, pinos e parâmetros de
 * configuração do projeto. Ao modificar hardware (pinos, endereços) ou
 * parâmetros de calibração, edite apenas este arquivo.
 * ============================================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ====
// LCD - Display 20x4 via I2C
// ====
#define LCD_ENDERECO      0x20    // Endereço I2C do display LCD
#define LCD_COLUNAS       20      // Número de colunas do display
#define LCD_LINHAS        4       // Número de linhas do display

// ====
// BOTÕES DE NAVEGAÇÃO
// ====
// Mapeamento: índice 0=Subir, 1=Descer, 2=Voltar, 3=Enter
// Os botões são lidos como ativo em LOW (INPUT_PULLUP)
#define DEBOUNCE_MS       250     // Tempo de debounce em milissegundos

const int BOTOES_PINOS[] = { 24, 22, 25, 23 };
const int NUM_BOTOES = sizeof(BOTOES_PINOS) / sizeof(BOTOES_PINOS[0]);

// Índices dos botões (para clareza no código)
#define BTN_SUBIR         0
#define BTN_DESCER        1
#define BTN_VOLTAR        2
#define BTN_ENTER         3

// ====
// ENTRADAS ANALÓGICAS - Potenciômetros de Controle
// ====
// Entrada 0-5V direto (sem divisor)
#define PIN_POT_RPM       A0      // Potenciômetro de regulagem de RPM (0-5V)
#define PIN_POT_VSS       A1      // Potenciômetro de regulagem de VSS (0-5V)
#define PIN_POT_MAP_HZ    A10     // Potenciômetro/sinal MAP PWM (0-5V)

// ====
// ENTRADAS ANALÓGICAS - Monitoramento de Sensores
// ====
// Divisor resistivo 5:1 → 0-25V entrada = 0-5V saída
// Pinos analógicos para leitura dos sensores da ECU
// Mapeamento real do hardware:
//   A2=ACT (div 5:1), A3=ECT (div 5:1), A4=TPS (div 5:1, sinal do sensor 0-5V),
//   A5=MAP (div 5:1), A6..A9=5V Sensores (div 5:1),
//   A11=Sonda1 (0-5V direto), A12=Sonda2 (0-5V direto)
// OBS: A10 = potenciômetro MAP PWM (PIN_POT_MAP_HZ), lido separadamente
#define PIN_TPS           A4
#define PIN_SONDA_1       A11
#define PIN_SONDA_2       A12

const uint8_t ANALOG_PINS[] = { A2, A3, A4, A5, A6, A7, A8, A9, A11, A12 };
const uint8_t NUM_ANALOG = sizeof(ANALOG_PINS) / sizeof(ANALOG_PINS[0]);

// Índices em analogValues[] / ANALOG_PINS[]
#define ANALOG_IDX_TPS     2
#define ANALOG_IDX_SONDA1  8
#define ANALOG_IDX_SONDA2  9

// Faixa elétrica do sinal do sensor TPS (antes do divisor no hardware)
#define TPS_SENSOR_MAX_V   5.0f

// ====
// SAÍDAS DE SINAL - Timers de Hardware
// ====
#define VSS_PIN_OC3A      5       // Saída hardware do Timer3 (OC3A) - sinal VSS
#define VACUUM_PIN        46      // Saída hardware do Timer5 (OC5A) - sinal vácuo/MAP

// ====
// RELÉS DE CONTROLE
// ====
#define RELE_HALL_MAGNETICO       50    // Relé: seleção Hall ou Magnético
#define RELE_5_12VOLTS            51    // Relé: tensão de saída Hall (5V ou 12V)
#define RELE_FASE_HALL_MAGNETICO  52    // Relé: fase Hall/Magnético

// ====
// CONSTANTES DE CALIBRAÇÃO
// ====
#define NUM_AMOSTRAS      12      // Quantidade de amostras na leitura analógica (média)
#define FATOR_VSS         4.3f    // Fator km/h → Hz (ex: 10 km/h = 43 Hz)

// Fatores de conversão ADC para Volts com divisor 5:1
// Divisor 5:1 significa: 25V entrada → 5V saída no ADC
// Fator = (5V / 1023 steps) × 5 (razão do divisor) = 0.02444 V/step
#define ADC_TO_VOLTS_5V   (5.0f / 1023.0f)        // Tensão no pino ADC (0-5V)
#define ADC_TO_VOLTS_25V  (5.0f / 1023.0f) * 5.0f // Entrada 0-25V com divisor 5:1

#define ADC_MAX_DIRECT    1023.0f   // ADC bruto no pino (0-5V)
#define ADC_MAX_DIV5      5115.0f   // ADC equivalente 0-25V (ACT, ECT, MAP)

/**
 * Recupera tensão 0-5V do sensor TPS quando há divisor 5:1 no hardware.
 * Ex.: 5V no sensor → ~1V no pino → ADC ~204 → retorna 5.0V
 */
inline float adcDiv5ToSensor5V(float adcRaw) {
  if (adcRaw < 0.0f) adcRaw = 0.0f;
  if (adcRaw > ADC_MAX_DIRECT) adcRaw = ADC_MAX_DIRECT;
  return adcRaw * ADC_TO_VOLTS_5V * 5.0f;
}

// Constantes do prescaler para Timer3 e Timer5
// NOTA: Arduino Mega 2560 roda a 16 MHz (16.000.000 Hz)
// O valor anterior (160000) estava errado por fator 100x, causando
// OCR absurdos e frequências incorretas nos Timer3 (VSS) e Timer5 (MAP).
const uint32_t F_CPU_HZ  = 16000000UL; // Frequência do cristal do ATmega2560 (16 MHz)
const uint16_t PRESCALER  = 64;         // Prescaler dos timers de hardware

// ====
// TIMERS - Períodos de Interrupção
// ====
// Timer1: cada interrupção avança 1 passo do padrão fônica/fase (ISR em ISR_Timer1.cpp).
// Período inicial ao entrar na simulação; depois o loop recalcula conforme RPM (A0).
#define TIMER1_PERIODO_US   10000   // 10 ms → 100 Hz até o potenciômetro ajustar

#define TIMER4_PERIODO_US   50000   // Timer4: leitura analógica (50 ms = 20 Hz)

// Limites aplicados no loop() antes de Timer1_SetPeriod() (ver ARQUITETURA.md → Timer1)
#define TIMER1_MIN_US       50UL     // ~20 kHz na ISR — teto alto de RPM
#define TIMER1_MAX_US       262000UL   // máx. OCR1A×4 µs (65535×4) — RPM baixo real

// ====
// MONITOR ANALÓGICO - Configuração de Exibição
// ====
#define MONITOR_VAR_COUNT   12    // Total de variáveis monitoradas
#define MONITOR_LINHAS_LCD  3     // Linhas disponíveis para scroll (linhas 1..3 do LCD)
#define LINHAS_VISIVEIS     3     // Elimina magic number
#define LCD_UPDATE_MS       100   // Intervalo mínimo entre atualizações do LCD (ms)
                    // 100ms = 10 Hz — responsivo para navegação,
                    // sem flicker graças ao smart-update (só reescreve mudanças)

// Labels das variáveis monitoradas (exibidos no LCD)
const char* const MONITOR_LABELS[MONITOR_VAR_COUNT] = {
  "VSS:", "MAP:", "MAP PWM:", "ACT:", "ECT:", "TPS:",
  "5_VOLTS(1):", "5_VOLTS(2):", "5_VOLTS(3):",
  "5_VOLTS(4):", "SONDA 1:", "SONDA 2:"
};

// ====
// NAVEGAÇÃO - Limites
// ====
#define PILHA_MENUS_MAX   16      // Profundidade máxima da pilha de navegação

// ====
// FUNÇÕES UTILITÁRIAS INLINE PARA VALIDAÇÃO
// ====

/**
 * Valida frequência contra NaN, Inf e faixa permitida.
 * Proteção contra valores inválidos
 */
inline float constrainFreq(float freq) {
  if (isnan(freq) || isinf(freq) || freq <= 0.0f) return 0.0f;
  return constrain(freq, 0.5f, 10000.0f);  // 0.5Hz a 10kHz
}

/**
 * Valida leitura ADC (0-1023).
 * Garante valor dentro do range válido
 */
inline int constrainADC(int adc) {
  return constrain(adc, 0, 1023);
}

/**
 * Valida temperatura (-40 a 150°C).
 * Garante faixa de temperatura razoável
 */
inline float constrainTemp(float temp) {
  return constrain(temp, -40.0f, 150.0f);
}

/**
 * Valida pressão em kPa (0 a 300 kPa).
 * Garante faixa de pressão razoável
 */
inline float constrainPressao(float pressao) {
  return constrain(pressao, 0.0f, 300.0f);
}

/**
 * Valida percentual (0 a 100%).
 * Garante faixa de porcentagem válida
 */
inline float constrainPercent(float pct) {
  return constrain(pct, 0.0f, 100.0f);
}

/**
 * Valida voltagem até 5V (entradas diretas: TPS, sonda lambda).
 */
inline float constrainVoltage5V(float voltage) {
  return constrain(voltage, 0.0f, 5.0f);
}

/**
 * Valida voltagem até 25V (entradas com divisor 5:1).
 */
inline float constrainVoltage(float voltage) {
  return constrain(voltage, 0.0f, 25.0f);
}

#endif // CONFIG_H
