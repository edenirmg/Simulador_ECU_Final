
/**
 * ============================================================================
 * Sensores.cpp — Módulo de Sensores e Timers de Hardware
 * ============================================================================
 *
 * Responsável por:
 *   - Conversão de sensores automotivos (NTC, TPS, MAP)
 *   - Leitura não-bloqueante dos pinos analógicos
 *   - Geração de sinal VSS via Timer3 (CTC, OC3A, pino 5)
 *   - Geração de sinal MAP/vácuo via Timer5 (CTC, OC5A, pino 46)
 *   - ISR Timer1: sinal fônica/hall/fase (PORTA bits 4-5)
 *   - ISR Timer4: flag de leitura periódica de sensores (20 Hz)
 *
 * VERSÃO 2.7 - CORRIGIDO: ADC sincronizado, recuperação de entrada original
 * ============================================================================
 */

#include "Sensores.h"
#include "Config.h"
#include "Sinais.h"
#include "Ajustes.h"

// ====
// DEFINIÇÃO DAS VARIÁVEIS GLOBAIS
// ====

volatile bool vssAtivo    = false;
volatile bool mapHzAtivo  = false;

volatile int analogValues[12];
volatile bool newData     = false;
volatile bool readSensors = false;   // Flag: ISR Timer4 pede leitura ao loop

float _potRpm = 0, _potVss = 0, _potMapHz = 0;
volatile float _rpm = 0, _vss = 0, _mapHz = 0, _rps = 0;
volatile float _tempo = TIMER1_PERIODO_US, _tempoVss = 0;
float _tensaoPotRpm = 0, _tensaoPotVss = 0;

// Tensões dos sensores analógicos
volatile float _tensaoMap = 0, _tensaoAct = 0, _tensaoEct = 0, _tensaoTps = 0;
volatile float _tensao5V_1 = 0, _tensao5V_2 = 0, _tensao5V_3 = 0, _tensao5V_4 = 0;
volatile float _tensaoMapPwm = 0;
volatile float _tensaoSonda1 = 0, _tensaoSonda2 = 0;

// Variáveis internas mantidas por compatibilidade
static float _tempoHego_1 = 0, _tempoKs = 0, _tensaoKs = 0;
static float _Sensor5Volts = 0, _tensao5Volts = 0;

// ====
// FUNÇÕES DE CONVERSÃO
// ====

float mmHgToFreq(float mmHg) {
  // Faixa linear: 0 mmHg → 159 Hz, 600 mmHg → 95.9 Hz
  float freq = 159.0f - (mmHg * (159.0f - 95.9f) / 600.0f);
  if (freq < 1.0f) freq = 1.0f;  // Proteção contra freq <= 0
  return freq;
}

float kmhToFreq(float kmh) {
  float freq = kmh * FATOR_VSS;
  if (freq < 0.0f) freq = 0.0f;
  return freq;
}

float calcTempoTimer1(float rpm) {
  if (rpm < 0.5f) {
    return (float)TIMER1_MAX_US;
  }

  if (_hall) {
    // 4 ISR por ciclo Hall; fator 10 → T_isr ≈ 10e6/rpm µs (comportamento calibrado)
    return calcPeriodo(rpm, 10.0f);
  }

  if (_fonica || _fonica1) {
    _rps = rpm / 60.0f;
    if (_falha > 0) {
      // Uma ISR por passo; _falha = passos por volta (74 para 36-1, 120 para 60-2, …)
      return calcPeriodo(_rps * (float)_falha, 1.0f);
    }
  }

  return (float)TIMER1_PERIODO_US;
}

// ====
// CONVERSÃO DE SENSORES AUTOMOTIVOS
// ====

/**
 * Tabela de interpolação NTC para sensores ACT/ECT automotivos.
 * Faixa: -40 °C a 150 °C
 */
static const float NTC_TABLE[][2] PROGMEM = {
  { 30,   150.0 },
  { 50,   140.0 },
  { 80,   130.0 },
  { 120,  120.0 },
  { 160,  110.0 },
  { 200,  100.0 },
  { 250,   90.0 },
  { 300,   80.0 },
  { 350,   70.0 },
  { 400,   60.0 },
  { 445,   50.0 },
  { 490,   40.0 },
  { 512,   35.0 },
  { 555,   25.0 },
  { 615,   10.0 },
  { 650,    0.0 },
  { 700,  -10.0 },
  { 750,  -25.0 },
  { 800,  -40.0 }
};

#define NTC_TABLE_SIZE  19

/**
 * Interpolação linear na tabela NTC com clamp em -40°C a 150°C.
 */
static float ntcInterpolate(float adcRaw) {
  // Clamp nos limites da tabela
  if (adcRaw <= pgm_read_float(&NTC_TABLE[0][0]))
    return 150.0f;
  if (adcRaw >= pgm_read_float(&NTC_TABLE[NTC_TABLE_SIZE - 1][0]))
    return -40.0f;

  // Busca o segmento correto e interpola
  for (uint8_t i = 0; i < NTC_TABLE_SIZE - 1; i++) {
    float adc0  = pgm_read_float(&NTC_TABLE[i][0]);
    float adc1  = pgm_read_float(&NTC_TABLE[i + 1][0]);

    if (adcRaw >= adc0 && adcRaw <= adc1) {
      float temp0 = pgm_read_float(&NTC_TABLE[i][1]);
      float temp1 = pgm_read_float(&NTC_TABLE[i + 1][1]);

      float frac = (adcRaw - adc0) / (adc1 - adc0);
      float resultado = temp0 + (temp1 - temp0) * frac;

      // Clamp extra de segurança
      if (resultado < -40.0f)   resultado = -40.0f;
      if (resultado > 150.0f)   resultado = 150.0f;
      return resultado;
    }
  }

  return 25.0f;
}

float converteACT(float adcRaw) {
  return ntcInterpolate(adcRaw);
}

float converteECT(float adcRaw) {
  return ntcInterpolate(adcRaw);
}

float converteTPS(float sensorVolts) {
  if (sensorVolts < 0.0f) sensorVolts = 0.0f;
  if (sensorVolts > TPS_SENSOR_MAX_V) sensorVolts = TPS_SENSOR_MAX_V;
  return (sensorVolts / TPS_SENSOR_MAX_V) * 100.0f;
}

float converteMAP(float adcRaw) {
  if (adcRaw < 0.0f)      adcRaw = 0.0f;
  if (adcRaw > ADC_MAX_DIV5) adcRaw = ADC_MAX_DIV5;  // divisor 5:1
  return (adcRaw / ADC_MAX_DIV5) * 101.325f;
}

// ====
// LEITURA ANALÓGICA COM MÉDIA
// ====

float lePorta(int portaAnalogica) {
  float total = 0;
  for (int i = 0; i < NUM_AMOSTRAS; i++) {
    total += analogRead(portaAnalogica);
    unsigned long inicio = micros();
    while (micros() - inicio < 5000UL) { }
  }
  return total / (float)NUM_AMOSTRAS;
}

// ====
// LEITURA NÃO-BLOQUEANTE DE SENSORES
// ====

/**
 * leSensoresAnalogicos() - Lê todos os pinos analógicos de monitoramento
 * 
 * ✅ CORRIGIDO v2.7:
 * - ACT, ECT, MAP: ADC equivalente com divisor 5:1 (× 5 → 0-5115)
 * - TPS (A4): divisor 5:1 no hardware, sinal do sensor 0-5V → _tensaoTps em Volts
 * - 5V Sensores (A6-A9): Volts na entrada 0-25V (divisor 5:1)
 * - Sondas (A11-A12): Volts 0-5V direto
 * - MAP PWM: Lê A10 e recupera ADC bruto
 */
void leSensoresAnalogicos() {
  // Lê A2..A9 (índices 0..7)
  for (uint8_t i = 0; i < ANALOG_IDX_SONDA1; i++) {
    analogValues[i] = analogRead(ANALOG_PINS[i]);
  }

  // Sondas — leitura separada, um analogRead() por pino
  analogValues[ANALOG_IDX_SONDA1] = analogRead(PIN_SONDA_1);
  analogValues[ANALOG_IDX_SONDA2] = analogRead(PIN_SONDA_2);

  // ============================================================
  // SENSORES COM DIVISOR 5:1 - Recuperar ADC equivalente entrada
  // ============================================================
  // Divisor 5:1: entrada 0-25V → saída 0-5V (ADC 0-1023)
  // Para recuperar o ADC da entrada original: multiplicar por 5
  // Função de conversão espera ADC equiv. entrada (0-5115)
  // ============================================================
  
  _tensaoAct    = (float)analogValues[0] * 5.0f;     // A2  → ACT (ADC equiv. div 5:1)
  _tensaoEct    = (float)analogValues[1] * 5.0f;     // A3  → ECT (ADC equiv. div 5:1)
  _tensaoTps    = adcDiv5ToSensor5V((float)analogValues[ANALOG_IDX_TPS]);  // A4 → Volts 0-5V
  _tensaoMap    = (float)analogValues[3] * 5.0f;     // A5  → MAP (ADC equiv. div 5:1)
  
  // ============================================================
  // SENSORES 5V (A6-A9) — divisor 5:1, entrada 0-25V
  // ============================================================
  _tensao5V_1   = (float)analogValues[4] * ADC_TO_VOLTS_25V;   // A6  → Volts
  _tensao5V_2   = (float)analogValues[5] * ADC_TO_VOLTS_25V;   // A7  → Volts
  _tensao5V_3   = (float)analogValues[6] * ADC_TO_VOLTS_25V;   // A8  → Volts
  _tensao5V_4   = (float)analogValues[7] * ADC_TO_VOLTS_25V;   // A9  → Volts
  
  // ============================================================
  // SONDAS LAMBDA (A11-A12) — 0-5V direto, sem divisor
  // ============================================================
  _tensaoSonda1 = (float)analogValues[ANALOG_IDX_SONDA1] * ADC_TO_VOLTS_5V;   // A11
  _tensaoSonda2 = (float)analogValues[ANALOG_IDX_SONDA2] * ADC_TO_VOLTS_5V;   // A12
  
  // ============================================================
  // POTENCIÔMETRO MAP (A10) - ADC bruto (0-1023)
  // ============================================================
  // Leitura separada (não está em ANALOG_PINS[])
  // Entrada 0-5V, sem divisor
  // Usado para calcular frequência MAP PWM via converteMAP()
  // ============================================================
  
  int mapPwm_raw = analogRead(PIN_POT_MAP_HZ);
  _tensaoMapPwm = (float)constrainADC(mapPwm_raw) * 5.0f;  // ADC equiv. entrada 0-25V

  newData = true;  // Sinaliza ao Display que há dados novos
}

// ====
// TIMER3 - GERAÇÃO DE SINAL VSS
// ====

void setupTimer3_VSS(float freq) {
  if (freq <= 0.0f || isnan(freq) || isinf(freq)) {
    stopTimer3();
    return;
  }

  float ocr_f = ((float)F_CPU_HZ / (2.0f * (float)PRESCALER * freq)) - 1.0f;
  
  if (ocr_f < 1.0f || ocr_f > 65535.0f) {
    stopTimer3();
    return;
  }
  
  uint16_t ocr = (uint16_t)ocr_f;

  const uint8_t CTC_P64 = (1 << WGM32) | (1 << CS31) | (1 << CS30);
  if ((TCCR3B & CTC_P64) == CTC_P64) {
    if (OCR3A != ocr) {
      OCR3A = ocr;
      if (TCNT3 > ocr) TCNT3 = 0;
    }
    return;
  }

  pinMode(VSS_PIN_OC3A, OUTPUT);
  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3  = 0;
  OCR3A  = ocr;
  TCCR3A = (1 << COM3A0);
  TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);

  vssAtivo = true;
}

void stopTimer3() {
  TCCR3A = 0;
  TCCR3B = 0;
  digitalWrite(VSS_PIN_OC3A, LOW);
  vssAtivo = false;
}

void setVssKmh(float kmh) {
  if (kmh <= 0.1f) {
    stopTimer3();
    return;
  }
  float f = kmhToFreq(kmh);
  if (f > 0.0f) {
    setupTimer3_VSS(f);
  } else {
    stopTimer3();
  }
}

// ====
// TIMER5 - GERAÇÃO DE SINAL MAP/VÁCUO
// ====

void setVacuumFreq(float freq) {
  if (freq <= 0.0f || isnan(freq) || isinf(freq)) {
    stopTimer5();
    return;
  }

  float ocr_f = ((float)F_CPU_HZ / (2.0f * (float)PRESCALER * freq)) - 1.0f;
  
  if (ocr_f < 1.0f || ocr_f > 65535.0f) {
    stopTimer5();
    return;
  }
  
  uint16_t ocr = (uint16_t)ocr_f;

  const uint8_t CTC_P64 = (1 << WGM52) | (1 << CS51) | (1 << CS50);
  if ((TCCR5B & CTC_P64) == CTC_P64) {
    if (OCR5A != ocr) {
      OCR5A = ocr;
      if (TCNT5 > ocr) TCNT5 = 0;
    }
    return;
  }

  pinMode(VACUUM_PIN, OUTPUT);
  TCCR5A = 0;
  TCCR5B = 0;
  TCNT5  = 0;
  OCR5A  = ocr;
  TCCR5A = (1 << COM5A0);
  TCCR5B = (1 << WGM52) | (1 << CS51) | (1 << CS50);

  mapHzAtivo = true;
}

void stopTimer5() {
  TCCR5A = 0;
  TCCR5B = 0;
  digitalWrite(VACUUM_PIN, LOW);
  mapHzAtivo = false;
}

// ====
// ROTINAS DE INTERRUPÇÃO (ISR)
// ====

static inline void escreverSaidaPorta(uint8_t mascara, bool altoLogico, bool inverter) {
  bool altoFisico = altoLogico ^ inverter;
  if (altoFisico) PORTA |= mascara;
  else            PORTA &= ~mascara;
}

/**
 * InterruptTimer1 - Gera os sinais de roda fônica e fase.
 * Mapeamento PORTA (Arduino Mega pinos 26-29):
 *   bit4 (PA4) → pin26 = Sinal Hall/Fônica
 *   bit5 (PA5) → pin27 = Sinal de Fase
 *
 * Importante: ao fim da volta fônica, reinicia _cP no mesmo tick e já gera
 * o 1º flanco. O reset “vazio” (_cP=0 sem XOR) alongava a falha em 1 passo
 * e parecia um corte irregular no osciloscópio.
 */
void InterruptTimer1() {
  const uint8_t MASK_FONICA = (1 << 4);
  const uint8_t MASK_FASE = (1 << 5);

  int cP = ++_cP;
  int cF = ++_cF;

  // --- Geração do Sinal Hall / Fônica ---
  if (_hall) {
    // Meio-ciclo a cada 2 ISR; reinicia sem passo morto
    if (cP >= 2) {
      PORTA ^= MASK_FONICA;
      _cP = 0;
    }
  } else if (_fonica || _fonica1) {
    const int limiteVolta = (_fonica1 && _falha1 > _falha) ? _falha1 : _falha;

    if (cP > limiteVolta) {
      _cP = 1;
      cP = 1;
    }

    if (cP <= _pulso) {
      PORTA ^= MASK_FONICA;
    } else if (cP <= _falha) {
      // Janela da falha (nível estável)
      escreverSaidaPorta(MASK_FONICA, true, inverterFonica);
    } else if (_fonica1 && cP <= _falha1) {
      escreverSaidaPorta(MASK_FONICA, false, inverterFonica);
    }
  }

  // --- Geração do Sinal de Fase ---
  switch (_faseOK) {
    case 0:
      escreverSaidaPorta(MASK_FASE, false, inverterFase);
      break;
    case 1:
      if (cF > _pI1D) {
        if (cF <= _l1D)  escreverSaidaPorta(MASK_FASE, true, inverterFase);
        else             escreverSaidaPorta(MASK_FASE, false, inverterFase);
      }
      break;
    case 2:
      if (cF > _pI1D) {
        if (cF <= _l1D)       escreverSaidaPorta(MASK_FASE, false, inverterFase);
        else if (cF <= _pI2D) escreverSaidaPorta(MASK_FASE, true, inverterFase);
        else if (cF <= _l2D)  escreverSaidaPorta(MASK_FASE, false, inverterFase);
        else                  escreverSaidaPorta(MASK_FASE, true, inverterFase);
      }
      break;
    case 3:
      if (cF > _pI1D) {
        if (cF <= _l1D)       escreverSaidaPorta(MASK_FASE, false, inverterFase);
        else if (cF <= _pI2D) escreverSaidaPorta(MASK_FASE, true, inverterFase);
        else if (cF <= _l2D)  escreverSaidaPorta(MASK_FASE, false, inverterFase);
        else if (cF <= _pI3D) escreverSaidaPorta(MASK_FASE, true, inverterFase);
        else                  escreverSaidaPorta(MASK_FASE, false, inverterFase);
      }
      break;
    case 4:
      if (cF > _pI1D) {
        if (cF <= _l1D)       escreverSaidaPorta(MASK_FASE, false, inverterFase);
        else if (cF <= _pI2D) escreverSaidaPorta(MASK_FASE, true, inverterFase);
        else if (cF <= _l2D)  escreverSaidaPorta(MASK_FASE, false, inverterFase);
        else if (cF <= _pI3D) escreverSaidaPorta(MASK_FASE, true, inverterFase);
        else if (cF <= _l3D)  escreverSaidaPorta(MASK_FASE, false, inverterFase);
        else if (cF <= _pI4D) escreverSaidaPorta(MASK_FASE, true, inverterFase);
        else if (cF <= _l4D)  escreverSaidaPorta(MASK_FASE, false, inverterFase);
        else                  escreverSaidaPorta(MASK_FASE, true, inverterFase);
      }
      break;
  }

  if (cF >= _fSF && _fSF > 0) _cF = 0;
}

/**
 * InterruptTimer4 - Sinaliza leitura periódica dos sensores.
 * Executa a cada TIMER4_PERIODO_US (50ms = 20 Hz).
 */
void InterruptTimer4() {
  readSensors = true;
}

