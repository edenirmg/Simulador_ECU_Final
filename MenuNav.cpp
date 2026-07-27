/**
 * ============================================================================
 * MenuNav.cpp — Navegação do Menu e Ativação da Simulação
 * ============================================================================
 *
 * Responsável por:
 *   - Detecção de botões com debounce
 *   - Navegação hierárquica (subir, descer, entrar, voltar) com pilha
 *   - Scroll no monitor analógico
 *   - AtivaMonitorAnalogico(): inicia simulação, configura sinal e timers
 * ============================================================================
 */

#include "MenuNav.h"
#include "Config.h"
#include "Display.h"
#include "MenuData.h"
#include "Sinais.h"
#include "Sensores.h"
#include "Reles.h"
#include "Timer1_Direct.h"  // ✅ NOVO: Include do Timer1 direto
#include "Ajustes.h"
#include <TimerFour.h>

// ====
// VARIÁVEIS DE NAVEGAÇÃO
// ====

const MenuItem *menuAtual       = menuPrincipal;  // Começa no menu principal
uint8_t selecao                 = 0;              // Item selecionado
uint8_t scrollMenu              = 0;              // Posição de scroll

// Pilha de navegação (para voltar aos menus anteriores)
static const MenuItem *pilhaMenus[PILHA_MENUS_MAX];
static uint8_t pilhaIndex = 0;

// ====
// DETECÇÃO DE BOTÕES COM DEBOUNCE
// ====

int botaoPressionadoIndice(const int *pins, int qtd, unsigned long debounce) {
  if (qtd <= 0) return -1;

  // Estados estáticos para debounce (máximo 20 botões)
  static unsigned long lastPressTime[20];
  static bool lastState[20];

  unsigned long agora = millis();

  for (int i = 0; i < qtd; i++) {
    bool leitura = (digitalRead(pins[i]) == LOW);  // Ativo em LOW (pull-up)

    if (leitura && !lastState[i] && (agora - lastPressTime[i] > debounce)) {
      lastPressTime[i] = agora;
      lastState[i] = true;
      return i;
    }
    if (!leitura) {
      lastState[i] = false;
    }
  }

  return -1;  // Nenhum botão pressionado
}

// ====
// NAVEGAÇÃO
// ====

void navegar(int dir) {
  if (modoAnalogico) {
    // --- Navegação no modo Monitor Analógico (scroll de sensores) ---
    int maxScroll = MONITOR_VAR_COUNT - MONITOR_LINHAS_LCD;
    if (maxScroll < 0) maxScroll = 0;

    int newIndex = scrollIndex + dir;
    if (newIndex < 0)         newIndex = 0;
    if (newIndex > maxScroll)  newIndex = maxScroll;

    if (newIndex != scrollIndex) {
      scrollIndex = newIndex;
      desenhaTelaMonitor();
      newData = true;   // Força atualização imediata dos valores
    }
  } else {
    // --- Navegação no modo Menu ---
    uint8_t total = contarItens(menuAtual);
    selecao = (selecao + dir + total) % total;

    // Usar constante LINHAS_VISIVEIS em vez de magic number
    // Ajuste do scroll para manter o item selecionado visível
    if (selecao < scrollMenu) {
      scrollMenu = selecao;
    } else if (selecao >= scrollMenu + LINHAS_VISIVEIS) {
      if (total <= LINHAS_VISIVEIS) {
        scrollMenu = 0;
      } else {
        scrollMenu = (total - LINHAS_VISIVEIS < selecao) ? (total - LINHAS_VISIVEIS) : (selecao - (LINHAS_VISIVEIS - 1));
      }
    }

    mostrarMenu(menuAtual, selecao, scrollMenu);
  }
}

void entrar() {
  MenuItem item = readMenuItem(&menuAtual[selecao]);

  if (item.submenu) {
    // Entra no submenu: empilha o menu atual
    if (pilhaIndex >= PILHA_MENUS_MAX) {
      Serial.println("ERRO: Pilha de menu cheia!");
      return;
    }
    pilhaMenus[pilhaIndex++] = menuAtual;
    
    menuAtual    = item.submenu;
    valParametro = item.parametro;
    selecao      = 0;
    scrollMenu   = 0;
    mostrarMenu(menuAtual, selecao, scrollMenu);

  } else if (item.acao) {
    // Executa a ação do item
    item.acao(item.parametro);

    if (modoEditorFase) return;

    // Se a ação ativou o monitor, desenha a tela do monitor
    if (modoAnalogico) {
      desenhaTelaMonitor();
      newData = true;
    } else {
      // Comportamento normal: volta a exibir o menu
      mostrarMenu(menuAtual, selecao, scrollMenu);
    }
  }
}

void voltar() {
  if (pilhaIndex > 0) {
    // Restaura o menu anterior
    menuAtual = pilhaMenus[--pilhaIndex];
    selecao    = 0;
    scrollMenu = 0;

    // Reseta estado da simulação
    sinalSelecionado = false;
    valParametro     = 0;
    _simulando       = false;
    modoAnalogico    = false;

    // ====================================================================
    // ✅ NOVO: Para todos os timers usando Timer1 direto
    // ====================================================================
    Timer1_Stop();      // Para Timer1
    Timer4.stop();      // Para Timer4
    stopTimer3();       // Para sinal VSS (Timer3 CTC)
    stopTimer5();       // Para sinal MAP/Vácuo (Timer5 CTC)
    
    vssAtivo    = false;
    mapHzAtivo  = false;
    readSensors = false;
    newData     = false;

    // Desliga relés
    releOff();

    // Exibe o menu restaurado
    mostrarMenu(menuAtual, selecao, scrollMenu);
  }
}

// ====
// ATIVAÇÃO DO MONITOR ANALÓGICO
// ====

void AtivaMonitorAnalogico(uint8_t param) {
  modoAnalogico = true;
  scrollIndex   = 0;

  // Desenha a tela fixa do monitor
  desenhaTelaMonitor();
  newData = true;

  // Seleciona sinal: param > 1 = índice direto (ex: Audi ATF = 6)
  //                  param ≤ 1 = ativação genérica, usa valParametro do submenu
  uint8_t sinalIdx = (param > 1) ? param : valParametro;
  SelecionaSin(sinalIdx);
  aplicarAjusteCustom(sinalIdx);
  registrarSinalSelecionado(sinalIdx);

  // Reset dos contadores de ISR antes de iniciar (evita pulsos espúrios)
  _cP = 0;
  _cF = 0;

  _simulando = true;

  // Lê potenciômetro e aplica período correto antes de iniciar Timer1
  {
    int potRpm_raw = analogRead(PIN_POT_RPM);
    float rpm = constrainFreq(obterRpmControle(potRpm_raw));
    _rpm   = rpm;
    _tempo = calcTempoTimer1(rpm);
  }

  Timer1_Stop();
  delay(10);

  Timer1_SetPeriod((unsigned long)_tempo);
  Timer1_Start();
  
  Timer4.start();   // Inicia Timer4 (leitura de sensores)

  // Leitura imediata — não esperar a primeira ISR do Timer4 (50 ms)
  leSensoresAnalogicos();
  readSensors = false;

  // Habilita geração de sinais VSS e MAP no loop()
  vssAtivo   = true;
  mapHzAtivo = true;

  readSensors = false;
  
  Serial.print(F("Timer1 iniciado - Periodo: "));
  Serial.print((unsigned long)_tempo);
  Serial.print(F(" us RPM: "));
  Serial.println((int)_rpm);
}
