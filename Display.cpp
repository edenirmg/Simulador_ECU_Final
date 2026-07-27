/**
 * ============================================================================
 * Display.cpp — Módulo de Controle do Display LCD 20x4 I2C
 * ============================================================================
 *
 * Responsável por:
 *   - Exibição de menus hierárquicos com scroll
 *   - Monitor analógico em tempo real (sensores e potenciômetros)
 *   - Smart-update: só reescreve posições que mudaram no LCD
 * ============================================================================
 */

#include "Display.h"
#include "Config.h"
#include "Sinais.h"
#include "Sensores.h"
#include "MenuData.h"
#include "Ajustes.h"
#include <avr/pgmspace.h>
#include <string.h>

// ====
// INSTÂNCIA DO LCD
// ====
LiquidCrystal_I2C lcd(LCD_ENDERECO, LCD_COLUNAS, LCD_LINHAS);

// ====
// VARIÁVEIS DE ESTADO
// ====
int scrollIndex    = 0;
bool modoAnalogico = false;

// ====
// INICIALIZAÇÃO
// ====

void inicializarDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

// ====
// EXIBIÇÃO DO MENU
// ====

void mostrarMenu(const MenuItem *menu, uint8_t pos, uint8_t scroll) {
  lcd.clear();
  char buffer[21];  // Buffer para leitura de strings PROGMEM (20 chars + '\0')

  // ---- Linha 0: Título do menu (centralizado) ----
  const char *titulo = NULL;
  for (uint8_t i = 0; i < titulosMenusCount; i++) { // Usa a contagem declarada em MenuData.cpp para evitar erro de sizeof em extern[]
    const MenuItem *m = (const MenuItem *)pgm_read_ptr(&titulosMenus[i].menu);

    if (m == menu) {
      const char *tituloPtr = (const char *)pgm_read_ptr(&titulosMenus[i].titulo);
      if (tituloPtr) {
        strncpy_P(buffer, tituloPtr, 20);
        buffer[20] = '\0';  // Garantia de null-termination
        titulo = buffer;
      }
      break;
    }
  }

  if (titulo) {
    int col = (LCD_COLUNAS - strlen(titulo)) / 2;
    if (col < 0) col = 0;
    lcd.setCursor(col, 0);
    lcd.print(titulo);
  }

  // ---- Linhas 1-3: Itens do menu ----
  uint8_t total = contarItens(menu);
  for (uint8_t i = 0; i < LINHAS_VISIVEIS; i++) {
    uint8_t idx = i + scroll;
    if (idx >= total) break;

    const MenuItem *itemPtr = &menu[idx];
    readMenuItemTexto(itemPtr, buffer, sizeof(buffer));

    lcd.setCursor(0, i + 1);
    lcd.print(idx == pos ? ">" : " ");
    lcd.print(buffer);
  }
}

// ====
// MONITOR ANALÓGICO - CONSTANTES DE LAYOUT
// ====

// Coluna onde começam os valores numéricos (após os labels)
#define VALOR_COL     12    // Coluna padrão para valores
#define VALOR_WIDTH    8    // Largura máxima da área de valor (20 - 12 = 8 chars)

// ====
// SMART-UPDATE: Buffers estáticos para evitar reescrita desnecessária no LCD
// ====
//
// Problema anterior: a cada ciclo, o código limpava a área com espaços e
// reescrevia o valor, causando um "flash" visível (branco→valor).
//
// Solução: formatar o valor em um char buffer de largura fixa (preenchido
// com espaços à direita), comparar com o buffer anterior e só enviar ao
// LCD se houve mudança. Uma única escrita padded elimina o clear.
//
// Layout dos buffers:
//   lcdBufRpm[7]   → "  1234" (6 chars padded + '\0')
//   lcdBufFase[4]  → "ON " ou "OFF" (3 chars padded + '\0')
//   lcdBufVal[3][VALOR_WIDTH+1] → valores das 3 linhas de sensor

static char lcdBufRpm[7]   = "";   // RPM: colunas 4..9 (6 chars)
static char lcdBufFase[4]  = "";   // Fase: colunas 16..18 (3 chars)
static char lcdBufVal[3][VALOR_WIDTH + 1] = { "", "", "" }; // Linhas 1-3

// Variável de throttling para atualização do LCD
static unsigned long ultimaAtualizacaoLCD = 0;

// ====
// FUNÇÕES AUXILIARES INTERNAS
// ====

/**
 * Preenche buf com espaços à direita até completar 'width' chars.
 * Garante terminação '\0'. Usado para evitar artefatos no LCD
 * sem precisar de um passo separado de "limpeza".
 */
static void padRight(char *buf, uint8_t width) {
  if (!buf) return;  // Validação NULL
  
  uint8_t len = strlen(buf);
  while (len < width) buf[len++] = ' ';
  buf[width] = '\0';
}

/**
 * Formata tensão em Volts sem usar %f no printf (AVR não imprime float por padrão).
 * Ex.: 4.85 → "4.85V"
 */
static void formataTensaoVolts(char *buf, uint8_t width, float valor) {
  if (!buf) return;

  if (isnan(valor) || isinf(valor)) {
    snprintf(buf, width + 1, "ERR");
    return;
  }

  long centi = (long)(valor * 100.0f + 0.5f);
  if (centi < 0)    centi = 0;
  if (centi > 9999) centi = 9999;

  int whole = (int)(centi / 100);
  int frac  = (int)(centi % 100);
  snprintf(buf, width + 1, "%d.%02dV", whole, frac);
}

/**
 * Escreve 'buf' no LCD na posição (col, linha) SOMENTE se diferir
 * do conteúdo em 'cache'. Se escreveu, atualiza 'cache'.
 * Retorna true se houve escrita, false se o cache já estava igual.
 */
static bool lcdWriteIfChanged(uint8_t col, uint8_t linha,
                               char *buf, char *cache, uint8_t width) {
  if (!buf || !cache) return false;  // Validação NULL
  
  // Garante padding antes da comparação
  padRight(buf, width);

  // Comparação byte-a-byte segura (não usa strncmp)
  bool changed = false;
  for (uint8_t i = 0; i < width; i++) {
    if (buf[i] != cache[i]) {
      changed = true;
      break;
    }
  }
  
  if (!changed) return false;  // Sem mudança

  // Houve mudança → escreve no LCD e atualiza cache
  lcd.setCursor(col, linha);
  lcd.print(buf);
  memcpy(cache, buf, width + 1);  // +1 para o '\0'
  return true;
}

/**
 * Obtém o valor convertido de uma variável monitorada pelo índice.
 * 
 * MAP PWM (idx 2) agora retorna kPa (não Volts!)
 * 
 * Mapeamento dos 12 índices (conforme MONITOR_LABELS[]):
 *   0  VSS:        Velocidade em km/h (calculada do potenciômetro)
 *   1  MAP:        Pressão absoluta em kPa — A5 (ADC → converteMAP)
 *   2  MAP PWM:    Pressão em kPa — A10 (ADC → converteMAP) ← AGORA EM kPa!
 *   3  ACT:        Temperatura do ar de admissão em °C — A2 (ADC → converteACT)
 *   4  ECT:        Temperatura do líquido de arrefecimento em °C — A3 (ADC → converteECT)
 *   5  TPS:        Posição da borboleta em % — A4 (Volts 0-5V → converteTPS)
 *   6  5_VOLTS(1): Tensão na entrada A6 — já em Volts (0-25V, leSensoresAnalogicos)
 *   7  5_VOLTS(2): Tensão na entrada A7
 *   8  5_VOLTS(3): Tensão na entrada A8
 *   9  5_VOLTS(4): Tensão na entrada A9
 *  10  SONDA 1:    Tensão na entrada A11 (0-5V) → _tensaoSonda1
 *  11  SONDA 2:    Tensão na entrada A12 (0-5V) → _tensaoSonda2
 */
static float obterValorMonitor(uint8_t idx) {
  switch (idx) {
    case 0:  return constrain(_vss, 0.0f, 200.0f);        // VSS km/h (mesmo valor usado no Timer3)
    case 1:  return constrainPressao(converteMAP(_tensaoMap));         // MAP: ADC → kPa
    case 2:  {                                                         // MAP PWM → kPa
      float mapPwm = converteMAP(_tensaoMapPwm);              // MAP PWM: ADC → kPa (não Volts!)
      return constrainPressao(mapPwm);
    }
    case 3:  return constrainTemp(converteACT(_tensaoAct));   // ACT: ADC → °C
    case 4:  return constrainTemp(converteECT(_tensaoEct));   // ECT: ADC → °C
    case 5:  return constrainPercent(converteTPS(_tensaoTps)); // TPS: ADC → %
    case 6:  return constrainVoltage(_tensao5V_1);
    case 7:  return constrainVoltage(_tensao5V_2);
    case 8:  return constrainVoltage(_tensao5V_3);
    case 9:  return constrainVoltage(_tensao5V_4);
    case 10: return constrainVoltage5V(_tensaoSonda1);
    case 11: return constrainVoltage5V(_tensaoSonda2);
    default: return 0.0f;
  }
}

/**
 * Formata o valor de uma variável monitorada em um buffer de texto.
 * O buffer é preenchido com espaços à direita até VALOR_WIDTH chars.
 *
 * Não toca no LCD — apenas formata a string.
 * O caractere ° do LCD HD44780 é o code 223 (0xDF).
 *
 * MAP PWM (idx 2) agora formata em kPa
 * 
 * Layout por tipo:
 *   idx 0  VSS:     "120 Km  "
 *   idx 1  MAP:     "101kPa  "
 *   idx 2  MAP PWM: "98kPa   "   ← AGORA EM kPa!
 *   idx 3  ACT:     "32\xDFC   "   (\xDF = ° no HD44780)
 *   idx 4  ECT:     "90\xDFC   "
 *   idx 5  TPS:     "75%     "
 *   idx 6-11:       "4.85V   "
 */
static void formataValor(char *buf, uint8_t idx, float valor) {
  if (!buf) return;  // Validação NULL
  
  if (isnan(valor) || isinf(valor)) {  // Proteção NaN/Inf
    snprintf(buf, VALOR_WIDTH + 1, "ERR");
    return;
  }
  
  switch (idx) {
    case 0:  // VSS: inteiro + " Km"
      snprintf(buf, VALOR_WIDTH + 1, "%d Km", (int)valor);
      break;

    case 1:  // MAP: inteiro + "kPa"
      snprintf(buf, VALOR_WIDTH + 1, "%dkPa", (int)valor);
      break;

    case 2:  // MAP PWM: inteiro + "kPa" (era Volts!)
      snprintf(buf, VALOR_WIDTH + 1, "%dkPa", (int)valor);
      break;

    case 3:  // ACT: inteiro + °C
    case 4:  // ECT: inteiro + °C
      snprintf(buf, VALOR_WIDTH + 1, "%d%cC", (int)valor, (char)223);
      break;

    case 5:  // TPS: inteiro + "%"
      snprintf(buf, VALOR_WIDTH + 1, "%d%%", (int)valor);
      break;

    default: // 5V / Sondas: tensão + "V" (sem %f — compatível com AVR)
      formataTensaoVolts(buf, VALOR_WIDTH, valor);
      break;
  }
  
  buf[VALOR_WIDTH] = '\0';  // Garante terminação
}

// ====
// MONITOR ANALÓGICO - TELA FIXA (labels)
// ====

void desenhaTelaMonitor() {
  lcd.clear();

  // Linha 0: RPM e Fase (fixos, sempre visíveis)
  lcd.setCursor(0, 0);
  lcd.print("RPM:");
  lcd.setCursor(11, 0);
  lcd.print("Fase:");

  // Linhas 1-3: Labels dos sensores conforme posição de scroll
  for (uint8_t i = 0; i < LINHAS_VISIVEIS; i++) {
    uint8_t idx = scrollIndex + i;
    lcd.setCursor(0, i + 1);
    if (idx < MONITOR_VAR_COUNT) {
      lcd.print(MONITOR_LABELS[idx]);
    } else {
      // Limpa a linha inteira se não houver mais variáveis
      for (uint8_t c = 0; c < LCD_COLUNAS; c++) lcd.print(' ');
    }
  }

  // Invalida caches por completo (lcd.clear() apaga o LCD; cache parcial impedia reescrita)
  memset(lcdBufRpm, 0, sizeof(lcdBufRpm));
  memset(lcdBufFase, 0, sizeof(lcdBufFase));
  memset(lcdBufVal, 0, sizeof(lcdBufVal));
  ultimaAtualizacaoLCD = 0;
}

// ====
// MONITOR ANALÓGICO - RESET DO THROTTLE (chamado pela navegação)
// ====

void resetThrottleLCD() {
  ultimaAtualizacaoLCD = 0;  // Permite atualização imediata no próximo ciclo
}

// ====
// MONITOR ANALÓGICO - ATUALIZAÇÃO DE VALORES (smart update)
// ====

void atualizaLCD() {
  if (modoEditorFase) return;
  if (!newData) return;
  unsigned long agora = millis();
  if (agora - ultimaAtualizacaoLCD < LCD_UPDATE_MS) return;
  ultimaAtualizacaoLCD = agora;
  newData = false;

  // --- Linha 0: RPM (colunas 4..9, 6 chars) ---
  {
    char buf[7];
    snprintf(buf, sizeof(buf), "%d", (int)_rpm);
    lcdWriteIfChanged(4, 0, buf, lcdBufRpm, 6);
  }

  // --- Linha 0: Fase (colunas 16..18, 3 chars) ---
  {
    char buf[4];
    strncpy(buf, (_faseOK != 0) ? "ON" : "OFF", sizeof(buf) - 1);
    buf[3] = '\0';
    lcdWriteIfChanged(16, 0, buf, lcdBufFase, 3);
  }

  // --- Linhas 1..3: Valores das variáveis visíveis ---
  for (uint8_t i = 0; i < LINHAS_VISIVEIS; i++) {
    uint8_t idx = scrollIndex + i;
    if (idx < MONITOR_VAR_COUNT) {
      float val = obterValorMonitor(idx);
      char buf[VALOR_WIDTH + 1];
      formataValor(buf, idx, val);
      lcdWriteIfChanged(VALOR_COL, i + 1, buf, lcdBufVal[i], VALOR_WIDTH);
    }
  }
}
