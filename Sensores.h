/**
 * ====
 * Sensores.h - Módulo de Leitura de Sensores e Timers de Hardware
 * ====
 * 
 * Responsável por:
 *   - Leituras analógicas (não-bloqueantes, feitas no loop principal)
 *   - Geração de sinal VSS via Timer3 (hardware CTC)
 *   - Geração de sinal MAP/vácuo via Timer5 (hardware CTC)
 *   - Lógica da ISR Timer1 (sinal fônica/hall/fase) — vetor em ISR_Timer1.cpp
 *   - Conversões de unidades (km/h→Hz, kPa→Hz, ADC→Tensão)
 *
 * NOTA SOBRE VOLATILE:
 *   Variáveis lidas/escritas tanto no loop() quanto em ISRs DEVEM ser volatile
 *   para impedir que o compilador as cache em registradores e ignore atualizações.
 * ====
 */

#ifndef SENSORES_H
#define SENSORES_H

#include <Arduino.h>
#include "Config.h"

// ====
// VARIÁVEIS GLOBAIS DE SENSORES
// ====

// Flags de controle dos módulos de sinal (lidas no loop, escritas no MenuNav)
extern volatile bool vssAtivo;       // Liga/desliga geração de sinal VSS
extern volatile bool mapHzAtivo;     // Liga/desliga geração de sinal MAP Hz

// Valores de leitura analógica
extern volatile int analogValues[12];
extern volatile bool newData;        // Flag: novos dados disponíveis para exibição
extern volatile bool readSensors;    // Flag: Timer4 ISR pede leitura de sensores ao loop

// Variáveis de controle e medição (escritas no loop)
extern float _potRpm, _potVss, _potMapHz;
extern volatile float _rpm, _vss, _mapHz, _rps;
extern volatile float _tempo, _tempoVss;
extern float _tensaoPotRpm, _tensaoPotVss;

// Tensões dos sensores analógicos (escritas no loop após leitura ADC)
extern volatile float _tensaoMap, _tensaoAct, _tensaoEct, _tensaoTps;  // TPS: Volts do sensor (0-5V)
extern volatile float _tensao5V_1, _tensao5V_2, _tensao5V_3, _tensao5V_4;
extern volatile float _tensaoMapPwm;
extern volatile float _tensaoSonda1, _tensaoSonda2;

// ====
// FUNÇÕES DE CONVERSÃO
// ====

/**
 * Converte leitura ADC (0-1023) para tensão escalada (mV).
 * @param leitura Valor bruto do ADC
 * @return Tensão em milivolts (0..~4887 mV)
 */
inline float analogToScaled(int leitura) {
  return (5.0f / 1023.0f) * leitura * 1000.0f;
}

/**
 * Converte frequência (Hz) em período do Timer1 (µs).
 *
 * @param freqHz Frequência desejada da ISR (Hz). Ex.: fônica → rps × _falha
 * @param fator  Escala opcional (Hall: 10 → período ≈ 10e6/rpm com valor=rpm)
 * @return Período em µs, limitado ao range do Timer1 (50 µs … 262 ms)
 */
inline float calcPeriodo(float freqHz, float fator = 1.0f) {
  if (freqHz <= 0.001f) return (float)TIMER1_MAX_US;
  float periodo = (fator / freqHz) * 1000000.0f;
  if (periodo < (float)TIMER1_MIN_US) periodo = (float)TIMER1_MIN_US;
  if (periodo > (float)TIMER1_MAX_US) periodo = (float)TIMER1_MAX_US;
  return periodo;
}

/**
 * Calcula o período Timer1 (_tempo) a partir do RPM e do sinal selecionado.
 * Usa _falha da tabela de sinais como passos por volta na fônica.
 */
float calcTempoTimer1(float rpm);

/**
 * Converte pressão em mmHg para frequência em Hz.
 * Faixa: 600 mmHg → 95.9 Hz, 0 mmHg → 159 Hz
 */
float mmHgToFreq(float mmHg);

/**
 * Converte velocidade em km/h para frequência em Hz.
 * Usa o FATOR_VSS definido em Config.h
 */
float kmhToFreq(float kmh);

// ====
// FUNÇÕES DE CONVERSÃO DE SENSORES
// ====

/**
 * Converte tensão ADC (0-1023) do sensor ACT (Air Charge Temperature) para °C.
 * Curva NTC típica automotiva (GM/Delphi):
 *   Usa interpolação linear por segmentos baseada na tabela de resistência
 *   padrão NTC 2.5kΩ @25°C com pull-up de 2.49kΩ (divisor resistivo 5V).
 * 
 * Faixa: 0°C a 100°C (valores fora travados nos limites)
 * @param adcRaw Valor bruto ADC (0-1023) — valor direto de analogValues[]
 * @return Temperatura em graus Celsius (0 - 100)
 */
float converteACT(float adcRaw);

/**
 * Converte tensão ADC (0-1023) do sensor ECT (Engine Coolant Temp) para °C.
 * Curva NTC típica automotiva (GM/Bosch):
 *   Mesma curva do ACT — ambos usam termistores NTC com característica similar.
 *   Em veículos reais, a curva pode diferir ligeiramente, mas para simulação
 *   a aproximação é válida.
 * 
 * Faixa: 0°C a 100°C (valores fora travados nos limites)
 * @param adcRaw Valor bruto ADC (0-1023) — valor direto de analogValues[]
 * @return Temperatura em graus Celsius (0 - 100)
 */
float converteECT(float adcRaw);

/**
 * Converte tensão do sensor TPS (0-5V em _tensaoTps) para % de abertura.
 * @param sensorVolts Tensão do sensor em Volts (0-5V), já compensada pelo divisor 5:1
 * @return Porcentagem de abertura (0.0 - 100.0%)
 */
float converteTPS(float adcRaw);

/**
 * Converte tensão ADC (0-1023) do sensor MAP para pressão em kPa.
 * Sensor MAP típico (GM 1-bar): 0V = 0 kPa (vácuo total), 5V = 101.325 kPa (1 atm).
 * Resposta linear com offset mínimo.
 * 
 * @param adcRaw Valor bruto ADC (0-1023) — valor direto de analogValues[]
 * @return Pressão absoluta em kPa (0 - 101.325)
 */
float converteMAP(float adcRaw);

// ====
// FUNÇÕES DE LEITURA
// ====

/**
 * Lê uma porta analógica com média de NUM_AMOSTRAS leituras.
 * ATENÇÃO: Usa busy-wait de ~5ms entre amostras (~60ms total).
 * NÃO usar dentro de ISRs ou no loop de simulação ativa.
 * Mantida para uso em calibração/setup quando timers estão parados.
 * 
 * @param portaAnalogica Pino analógico a ser lido
 * @return Valor médio da leitura (0.0 - 1023.0)
 */
float lePorta(int portaAnalogica);

/**
 * Realiza leitura não-bloqueante de todos os sensores analógicos.
 * Lê A2-A9, A11-A12 e A10 (pot MAP). TPS: div 5:1; sondas: 0-5V direto.
 * Chamada no loop() principal quando readSensors == true (setado pela ISR Timer4).
 * Seta newData=true para sinalizar ao Display que há dados novos.
 */
void leSensoresAnalogicos();

// ====
// CONTROLE DO TIMER3 - VSS (Velocidade)
// ====

/**
 * Configura o Timer3 para gerar sinal de VSS na frequência especificada.
 * Utiliza modo CTC com toggle automático no pino OC3A.
 * Protegido: se freq <= 0 ou OCR fora do range, para o timer.
 * 
 * @param freq Frequência desejada em Hz (deve ser > 0)
 */
void setupTimer3_VSS(float freq);

/**
 * Para o Timer3 e coloca o pino VSS em LOW.
 */
void stopTimer3();

/**
 * Atualiza o sinal VSS a partir da velocidade em km/h.
 * Se kmh <= 0.1, para o timer automaticamente.
 * 
 * @param kmh Velocidade em km/h
 */
void setVssKmh(float kmh);

// ====
// CONTROLE DO TIMER5 - MAP/Vácuo
// ====

/**
 * Configura o Timer5 para gerar sinal de MAP/vácuo na frequência especificada.
 * Utiliza modo CTC com toggle automático no pino OC5A.
 * Protegido: se freq <= 0 ou OCR fora do range, para o timer.
 * 
 * @param freq Frequência desejada em Hz (deve ser > 0)
 */
void setVacuumFreq(float freq);

/**
 * Para o Timer5 e coloca o pino de vácuo em LOW.
 */
void stopTimer5();

// ====
// ROTINAS DE INTERRUPÇÃO
// ====

/**
 * Corpo da ISR do Timer1 — chamada por ISR(TIMER1_COMPA_vect) em ISR_Timer1.cpp.
 * Período entre chamadas: OCR1A (ver Timer1_SetPeriod em Timer1_Direct.cpp).
 * Manipula PORTA bit4 (pino 26, fônica/Hall) e bit5 (pino 27, fase).
 * Manter leve (~2–5 µs); sem analogRead/Serial/LCD. Ver ARQUITETURA.md → Timer1.
 */
void InterruptTimer1();

/**
 * ISR do Timer4 - Sinaliza que é hora de ler sensores.
 * NOTA: NÃO faz analogRead() dentro da ISR (bloquearia Timer1 por ~1.1ms).
 * Apenas seta readSensors=true para que o loop() faça a leitura.
 */
void InterruptTimer4();

#endif // SENSORES_H
