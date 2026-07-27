/**
 * ============================================================================
 * Tipos.h - Definições de Estruturas de Dados
 * ============================================================================
 * 
 * Contém todas as structs utilizadas no projeto:
 *   - MenuItem:    Item de menu com submenu, texto, ação e parâmetro
 *   - MenuTitulo:  Associa um ponteiro de menu ao seu título
 *   - SinalConfig: Configuração de sinal específico de um veículo
 *   - BaseConfig:  Configuração base de um tipo de roda fônica/hall
 * 
 * As macros MENU_ITEM e MENU_END facilitam a declaração de menus em PROGMEM.
 * ============================================================================
 */

#ifndef TIPOS_H
#define TIPOS_H

#include <Arduino.h>

// ============================================================================
// ESTRUTURA DO MENU
// ============================================================================

/**
 * MenuItem - Representa um item individual no sistema de menus.
 * 
 * Campos:
 *   submenu   - Ponteiro para array de MenuItem (submenu), NULL se não há submenu
 *   texto     - Ponteiro para string em PROGMEM com o texto exibido no LCD
 *   acao      - Ponteiro para função executada ao selecionar (recebe parametro)
 *   parametro - Valor passado para a função de ação (ex: índice do sinal)
 * 
 * Exemplo de uso:
 *   Se submenu != NULL → ao pressionar ENTER, entra no submenu
 *   Se acao != NULL    → ao pressionar ENTER, executa acao(parametro)
 */
struct MenuItem {
  const struct MenuItem *submenu;   // Ponteiro para submenu (NULL = sem submenu)
  const char *texto;                // Texto do item (armazenado em PROGMEM)
  void (*acao)(uint8_t);            // Função de callback ao selecionar
  uint8_t parametro;                // Parâmetro passado ao callback
};

/**
 * MenuTitulo - Associa um menu ao seu título para exibição.
 * 
 * Usado para buscar o título correto ao exibir qualquer nível de menu.
 */
struct MenuTitulo {
  const MenuItem *menu;     // Ponteiro para o array de itens do menu
  const char *titulo;       // Título exibido na linha 0 do LCD (PROGMEM)
};

// Macros para facilitar a declaração de menus em PROGMEM
#define MENU_ITEM(sub, text, func, parm)  { sub, text, func, parm }
#define MENU_END                          { NULL, NULL, NULL, 0 }

// ============================================================================
// CONFIGURAÇÃO DE SINAIS
// ============================================================================

/**
 * SinalConfig - Configuração completa de um sinal de veículo específico.
 * 
 * Cada veículo possui um tipo de sinal base (tipSin) e opcionalmente
 * uma configuração de fase (tipFse) com até 4 dentes de referência.
 * 
 * Campos:
 *   tipSin - Tipo de sinal base (1=Hall, 2=36-1, 3=36-2, 4=44-4, 5=60-2, 6=60-1+1)
 *   tipFse - Tipo/quantidade de fases (0=sem fase, 1..4=nº de dentes de fase)
 *   dnt1..dnt4 - Posição inicial de cada dente de fase (em pulsos)
 *   lrd1..lrd4 - Largura de cada dente de fase (posição final em pulsos)
 * 
 * Para adicionar um novo veículo:
 *   1. Escolha o tipSin adequado (veja BaseConfig)
 *   2. Defina tipFse = nº de dentes de fase (0 se não houver)
 *   3. Preencha dnt/lrd para cada dente de fase
 *   4. Adicione a entrada na tabela 'sinais' em Sinais.cpp
 */
struct SinalConfig {
  byte tipSin;                      // Tipo de sinal base (índice em baseSinais[])
  byte tipFse;                      // Quantidade de fases (dentes de referência)
  int dnt1, lrd1;                   // Dente 1: posição inicial, largura
  int dnt2, lrd2;                   // Dente 2: posição inicial, largura
  int dnt3, lrd3;                   // Dente 3: posição inicial, largura
  int dnt4, lrd4;                   // Dente 4: posição inicial, largura
};

/**
 * BaseConfig - Configuração base de cada tipo de roda fônica.
 * 
 * Define as características fundamentais de cada padrão de roda dentada:
 *   tipo     - Identificador do tipo (1..6)
 *   pulso    - Número de pulsos por ciclo (ex: 72 para 36-1, 116 para 60-2)
 *   falha    - Posição da primeira falha (dente ausente)
 *   falha1   - Posição da segunda falha (se houver, ex: 60-1+1)
 *   hall     - true se é sinal Hall contínuo (sem roda fônica)
 *   fonica36 - true se é roda de 36 dentes
 *   fonica44 - true se é roda de 44 dentes
 *   fonica62 - true se é roda de 60+ dentes
 *   fonica   - true se usa padrão fônica padrão (1 falha)
 *   fonica1  - true se usa padrão fônica com 2 falhas
 *   Ccf      - Comprimento do ciclo de fase (total de pulsos por ciclo completo)
 */
struct BaseConfig {
  byte tipo;
  int pulso;
  int falha;
  int falha1;
  bool hall;
  bool fonica36;
  bool fonica44;
  bool fonica62;
  bool fonica;
  bool fonica1;
  int Ccf;        // Comprimento do Ciclo de Fase
};

#endif // TIPOS_H
