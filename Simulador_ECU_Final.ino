/**
 * ============================================================================
 * Simulador_ECU_Final.ino — Arquivo Principal do Simulador de ECU
 * ============================================================================
 *
 *  Descrição:
 *    Simulador de ECU (Electronic Control Unit) para Arduino Mega 2560.
 *    Gera sinais de roda fônica (Hall contínuo, 36-1, 36-2, 44-4, 60-2,
 *    60-1+1) com fase configurável para 55 sinais de módulos.
 *    Inclui geração de sinais VSS e MAP por hardware timer, leitura de
 *    sensores analógicos e interface de menu em display LCD 20x4 I2C.
 *
 *  Autor:  Edemir Marques Gelocha
 *  Versão: 2.8.0
 *
 *  Veja CHANGELOG.md para histórico completo de alterações.
 *  Veja README.md para instruções de instalação e uso.
 *  Veja ARQUITETURA.md para a estrutura modular do projeto.
 * ============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>
#include "Timer1_Direct.h"     // ✅ NOVO: Controle direto do Timer1
#include <TimerFour.h>
#include <TimerFive.h>

#include "Config.h"
#include "Tipos.h"
#include "Sinais.h"
#include "Sensores.h"
#include "Display.h"
#include "MenuData.h"
#include "MenuNav.h"
#include "Reles.h"
#include "Ajustes.h"

// ====
// VARIÁVEIS GLOBAIS ADICIONAIS
// ====
static unsigned long ultimaAtualizacaoTimer = 0;
static const unsigned long INTERVALO_ATUALIZA_TIMER = 50UL;  // 50ms
static float ultimoRpmAtualizado = -1.0f;

// ====
// SETUP - Inicialização do Sistema
// ====
void setup() {
  Serial.begin(9600);
  Serial.println("Simulador ECU v2.8 - Inicializando...");

  _simulando = false;
  _tempo     = TIMER1_PERIODO_US;

  inicializarDisplay();
  Serial.println("Display inicializado");

  for (int i = 0; i < NUM_BOTOES; ++i) {
    pinMode(BOTOES_PINOS[i], INPUT_PULLUP);
  }
  Serial.println("Botoes configurados");

  inicializarReles();
  Serial.println("Reles inicializados");

  DDRA |= B11110000;
  Serial.println("PORTA configurada");

  // ====================================================================
  // ✅ NOVO: Usar Timer1 direto (sem biblioteca TimerOne)
  // ====================================================================
  Timer1_Initialize();
  Serial.println("Timer1 configurado (controle direto)");

  Timer4.initialize(TIMER4_PERIODO_US);
  Timer4.attachInterrupt(InterruptTimer4);
  Timer4.stop();
  Serial.println("Timer4 configurado");

  inicializarAjustes();
  Serial.println("Ajustes EEPROM carregados");

  mostrarMenu(menuAtual, selecao, scrollMenu);
  Serial.println("Inicializacao concluida - PRONTO");
}

// ====
// LOOP - Ciclo Principal
// ====
void loop() {
  processarSerial();

  int idx = botaoPressionadoIndice(BOTOES_PINOS, NUM_BOTOES, DEBOUNCE_MS);

  if (idx >= 0) {
    if (modoEditorFase) {
      switch (idx) {
        case BTN_SUBIR:   editorFaseNavegar(1);  break;
        case BTN_DESCER:  editorFaseNavegar(-1); break;
        case BTN_VOLTAR:  editorFaseCancelar();  break;
        case BTN_ENTER:   editorFaseAvancar();   break;
      }
    } else {
      switch (idx) {
        case BTN_SUBIR:   navegar(-1);  break;
        case BTN_DESCER:  navegar(1);   break;
        case BTN_VOLTAR:  voltar();     break;
        case BTN_ENTER:   entrar();     break;
      }
    }
  }

  // --- Leitura periódica de sensores (solicitada pela ISR Timer4 a 20 Hz) ---
  if (readSensors) {
    readSensors = false;
    leSensoresAnalogicos();
  }

  // --- Modo de simulação ativo ---
  if (_simulando) {
    // ====================================================================
    // LEITURA DOS POTENCIÔMETROS DE CONTROLE
    // ====================================================================
    // A0 (RPM):    ADC 0-1023 → RPM 0-5100
    // A1 (VSS):    ADC 0-1023 → km/h 0-200
    // A10 (MAP Hz): ADC 0-1023 → mmHg 0-600
    
    int potRpm_raw = analogRead(PIN_POT_RPM);
    int potVss_raw = analogRead(PIN_POT_VSS);
    int potMap_raw = analogRead(PIN_POT_MAP_HZ);
    
    _potRpm   = (float)constrainADC(potRpm_raw);
    _potVss   = (float)constrainADC(potVss_raw);
    _potMapHz = (float)constrainADC(potMap_raw);

    // ====================================================================
    // CÁLCULO DO RPM (0-5100)
    // ====================================================================
    // Mapeamento linear: ADC 0-1023 → RPM 0-5100
    float rpmCalculado = obterRpmControle(potRpm_raw);
    _rpm = constrainFreq(rpmCalculado);

    // ====================================================================
    // CÁLCULO DO PERÍODO DO TIMER1 (frequência da ISR conforme tipo de roda)
    // ====================================================================
    _tempo = calcTempoTimer1(_rpm);

    // ====================================================================
    // ATUALIZAÇÃO DO TIMER1 (throttle 50 ms; sempre aplica se RPM mudou > 1%)
    // ====================================================================
    unsigned long agora = millis();
    if (agora - ultimaAtualizacaoTimer >= INTERVALO_ATUALIZA_TIMER) {
      ultimaAtualizacaoTimer = agora;

      float deltaPct = abs(_rpm - ultimoRpmAtualizado) / max(_rpm + 1.0f, 1.0f);

      if (obterModoRpm() == RPM_VARREDURA || deltaPct > 0.01f || ultimoRpmAtualizado < 0.0f) {
        Timer1_SetPeriod((unsigned long)_tempo);
        ultimoRpmAtualizado = _rpm;
        
        // Debug a cada segundo
        static unsigned long lastDebugTimer = 0;
        if (millis() - lastDebugTimer > 1000) {
          lastDebugTimer = millis();
          float freqHz = 1000000.0f / _tempo;
          Serial.print(F("RPM="));
          Serial.print((int)_rpm);
          Serial.print(F(" Periodo="));
          Serial.print((unsigned long)_tempo);
          Serial.print(F("µs ("));
          Serial.print(freqHz, 1);
          Serial.println(F(" Hz)"));
        }
      }
    }

    // ====================================================================
    // CÁLCULO DO VSS (0-200 km/h)
    // ====================================================================
    float kmh = (_potVss / 1023.0f) * 200.0f;
    _vss = kmh;

    if (vssAtivo) {
      setVssKmh(kmh);
    } else {
      stopTimer3();
    }

    // ====================================================================
    // CÁLCULO DO MAP Hz (0-600 mmHg)
    // ====================================================================
    float mmHg = (_potMapHz / 1023.0f) * 600.0f;
    _mapHz = mmHg;
    float freqmmH = mmHgToFreq(mmHg);

    if (mapHzAtivo) {
      setVacuumFreq(freqmmH);
    } else {
      stopTimer5();
    }

    // --- Atualiza valores no LCD ---
    atualizaLCD();
  }
}
