/**
 * ============================================================================
 * Display.h - Módulo de Controle do Display LCD
 * ============================================================================
 * 
 * Gerencia toda a interação com o display LCD 20x4 I2C:
 *   - Inicialização do LCD
 *   - Exibição do menu (título + itens com cursor)
 *   - Tela do monitor analógico (RPM, sensores, etc.)
 *   - Atualização periódica dos valores no monitor
 *   - Scroll vertical no monitor analógico
 * ============================================================================
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Tipos.h"

// ============================================================================
// OBJETO LCD (global, acessível por outros módulos)
// ============================================================================
extern LiquidCrystal_I2C lcd;

// ============================================================================
// VARIÁVEIS DE ESTADO DO DISPLAY
// ============================================================================
extern int scrollIndex;         // Posição atual do scroll no monitor analógico
extern bool modoAnalogico;      // true = exibindo monitor, false = exibindo menu

// ============================================================================
// FUNÇÕES PÚBLICAS
// ============================================================================

/**
 * Inicializa o LCD (I2C, backlight, limpa tela).
 * Deve ser chamada no setup().
 */
void inicializarDisplay();

/**
 * Exibe um menu no LCD com título centralizado e itens com cursor.
 * 
 * Linha 0: Título do menu (centralizado)
 * Linhas 1-3: Itens do menu (com ">" no item selecionado)
 * 
 * @param menu    Ponteiro para o array de itens do menu (PROGMEM)
 * @param pos     Índice do item atualmente selecionado
 * @param scroll  Posição de scroll (primeiro item visível)
 */
void mostrarMenu(const MenuItem *menu, uint8_t pos, uint8_t scroll);

/**
 * Desenha a tela fixa do monitor analógico.
 * Linha 0: "RPM: xxxx  Fase: ON/OFF"
 * Linhas 1-3: Labels dos sensores conforme scrollIndex
 * 
 * Chamada ao entrar no modo analógico ou ao mudar o scroll.
 */
void desenhaTelaMonitor();

/**
 * Atualiza os valores numéricos no monitor analógico.
 * Só executa se newData == true (leitura ADC ou redraw do monitor).
 * Atualiza RPM, fase e os valores das variáveis visíveis.
 */
void atualizaLCD();

#endif // DISPLAY_H
