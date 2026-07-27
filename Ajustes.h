#ifndef AJUSTES_H
#define AJUSTES_H

#include <Arduino.h>
#include "Tipos.h"

enum ModoRpm : uint8_t {
  RPM_POTENCIOMETRO = 0,
  RPM_FIXO = 1,
  RPM_VARREDURA = 2
};

extern volatile bool inverterFonica;
extern volatile bool inverterFase;
extern bool modoEditorFase;

void inicializarAjustes();
void processarSerial();

float obterRpmControle(int adcPot);
ModoRpm obterModoRpm();
uint16_t obterRpmFixo();

void acaoModoRpm(uint8_t parametro);
void acaoAbrirEditorFase(uint8_t parametro);
void acaoInverterFonica(uint8_t parametro);
void acaoInverterFase(uint8_t parametro);
void acaoRestaurarFase(uint8_t parametro);

void editorFaseNavegar(int direcao);
void editorFaseAvancar();
void editorFaseCancelar();

void registrarSinalSelecionado(uint8_t sinal);
void aplicarAjusteCustom(uint8_t sinal);
void imprimirStatus();
void imprimirDumpFase();

#endif
