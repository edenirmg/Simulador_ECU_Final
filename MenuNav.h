/**
 * ============================================================================
 * MenuNav.h - Módulo de Navegação do Menu
 * ============================================================================
 * 
 * Gerencia toda a lógica de navegação do sistema de menus:
 *   - Detecção de botões com debounce
 *   - Navegação vertical (subir/descer) com scroll
 *   - Entrada em submenus e execução de ações
 *   - Retorno ao menu anterior (pilha de navegação)
 *   - Ativação/desativação do modo monitor analógico
 * ============================================================================
 */

#ifndef MENU_NAV_H
#define MENU_NAV_H

#include <Arduino.h>
#include "Config.h"
#include "Tipos.h"

// ============================================================================
// VARIÁVEIS DE ESTADO DA NAVEGAÇÃO
// ============================================================================

extern const MenuItem *menuAtual;       // Menu sendo exibido atualmente
extern uint8_t selecao;                 // Índice do item selecionado
extern uint8_t scrollMenu;              // Posição de scroll (primeiro item visível)

// ============================================================================
// FUNÇÕES DE NAVEGAÇÃO
// ============================================================================

/**
 * Detecta qual botão foi pressionado com debounce.
 * 
 * @param pins     Array de pinos dos botões
 * @param qtd      Quantidade de botões
 * @param debounce Tempo de debounce em ms (padrão: DEBOUNCE_MS)
 * @return Índice do botão pressionado (0..qtd-1), ou -1 se nenhum
 */
int botaoPressionadoIndice(const int *pins, int qtd, unsigned long debounce = DEBOUNCE_MS);

/**
 * Navega no menu ou no monitor analógico.
 * 
 * No modo menu: move o cursor para cima/baixo com scroll automático.
 * No modo monitor: faz scroll das variáveis monitoradas.
 * 
 * @param dir Direção (-1 = subir, +1 = descer)
 */
void navegar(int dir);

/**
 * Entra no submenu ou executa a ação do item selecionado.
 * 
 * Se o item tem submenu → empilha o menu atual e entra no submenu.
 * Se o item tem ação → executa a função de callback com o parâmetro.
 */
void entrar();

/**
 * Volta ao menu anterior (desempilha da pilha de navegação).
 * 
 * Ao voltar:
 *   - Para todos os timers de simulação
 *   - Desliga os relés
 *   - Desativa o modo monitor analógico
 *   - Restaura o menu anterior
 */
void voltar();

/**
 * Ativa o modo monitor analógico.
 * Inicia a simulação do sinal selecionado e exibe leituras no LCD.
 * 
 * @param param Parâmetro (não utilizado diretamente, mantido por compatibilidade)
 */
void AtivaMonitorAnalogico(uint8_t param);

#endif // MENU_NAV_H
