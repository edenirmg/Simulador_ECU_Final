#include "Ajustes.h"

#include <EEPROM.h>
#include <avr/interrupt.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "Config.h"
#include "Display.h"
#include "MenuNav.h"
#include "Sensores.h"
#include "Sinais.h"
#include "Timer1_Direct.h"

static const uint32_t EEPROM_MAGIC = 0x45435532UL; // "ECU2"
static const uint8_t EEPROM_VERSION = 1;
static const int EEPROM_ENDERECO = 0;

struct ConfigPersistente {
  uint32_t magic;
  uint8_t version;
  uint8_t ultimoSinal;
  uint8_t modoRpm;
  uint8_t flags;
  uint16_t rpmFixo;
  uint16_t sweepMin;
  uint16_t sweepMax;
  uint16_t sweepPasso;
  uint8_t sinalCustom;
  uint8_t customValido;
  SinalConfig custom;
  uint16_t checksum;
};

static ConfigPersistente cfg;
static uint16_t rpmSweepAtual = 1000;
static bool sweepSubindo = true;
static unsigned long ultimaEtapaSweep = 0;

volatile bool inverterFonica = false;
volatile bool inverterFase = false;
bool modoEditorFase = false;

/**
 * Mostra no LCD (e Serial) o que o usuário acabou de escolher.
 * Bloqueia ~1 s para leitura antes do menu ser redesenhado.
 */
static void informarAjuste(const __FlashStringHelper *titulo,
                           const __FlashStringHelper *detalhe,
                           bool persistiu = true) {
  Serial.print(F("Ajuste: "));
  Serial.print(titulo);
  Serial.print(F(" -> "));
  Serial.println(detalhe);

  // Durante simulação/monitor, só confirma no Serial (sem travar o loop)
  if (_simulando || modoAnalogico) return;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(persistiu ? F("AJUSTE OK") : F("AVISO"));
  lcd.setCursor(0, 1);
  lcd.print(titulo);
  lcd.setCursor(0, 2);
  lcd.print(detalhe);
  lcd.setCursor(0, 3);
  lcd.print(persistiu ? F("Salvo na EEPROM") : F("Nada gravado"));
  delay(1000);
}

static void informarAjusteNum(const __FlashStringHelper *titulo,
                              const __FlashStringHelper *prefixo,
                              int valor,
                              const __FlashStringHelper *sufixo,
                              bool persistiu = true) {
  Serial.print(F("Ajuste: "));
  Serial.print(titulo);
  Serial.print(F(" -> "));
  Serial.print(prefixo);
  Serial.print(valor);
  if (sufixo) Serial.print(sufixo);
  Serial.println();

  if (_simulando || modoAnalogico) return;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(persistiu ? F("AJUSTE OK") : F("AVISO"));
  lcd.setCursor(0, 1);
  lcd.print(titulo);
  lcd.setCursor(0, 2);
  lcd.print(prefixo);
  lcd.print(valor);
  if (sufixo) lcd.print(sufixo);
  lcd.setCursor(0, 3);
  lcd.print(persistiu ? F("Salvo na EEPROM") : F("Nada gravado"));
  delay(1000);
}

static SinalConfig editorCfg;
static SinalConfig editorBackup;
static uint8_t editorCampo = 0;

static uint16_t calcularChecksum(const ConfigPersistente &dados) {
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&dados);
  uint16_t soma = 0x5A5A;
  for (size_t i = 0; i < offsetof(ConfigPersistente, checksum); ++i) {
    soma = (uint16_t)((soma << 5) | (soma >> 11));
    soma ^= bytes[i];
  }
  return soma;
}

static void valoresPadrao() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.magic = EEPROM_MAGIC;
  cfg.version = EEPROM_VERSION;
  cfg.ultimoSinal = 5;
  cfg.modoRpm = RPM_POTENCIOMETRO;
  cfg.rpmFixo = 2000;
  cfg.sweepMin = 800;
  cfg.sweepMax = 5000;
  cfg.sweepPasso = 50;
}

static void salvarConfig() {
  cfg.flags = (inverterFonica ? 0x01 : 0x00) |
              (inverterFase ? 0x02 : 0x00);
  cfg.checksum = calcularChecksum(cfg);
  EEPROM.put(EEPROM_ENDERECO, cfg); // put usa update: só grava bytes alterados
}

static bool configValida() {
  if (cfg.magic != EEPROM_MAGIC || cfg.version != EEPROM_VERSION) return false;
  if (cfg.checksum != calcularChecksum(cfg)) return false;
  if (cfg.ultimoSinal == 0 || cfg.ultimoSinal > NUM_SINAIS) return false;
  if (cfg.modoRpm > RPM_VARREDURA) return false;
  if (cfg.rpmFixo < 100 || cfg.rpmFixo > 10000) return false;
  if (cfg.sweepMin < 100 || cfg.sweepMax > 10000 || cfg.sweepMin >= cfg.sweepMax) return false;
  if (cfg.sweepPasso == 0 || cfg.sweepPasso > 1000) return false;
  if (cfg.customValido && (cfg.sinalCustom == 0 || cfg.sinalCustom > NUM_SINAIS)) return false;
  if (cfg.customValido) {
    if (cfg.custom.tipSin < 1 || cfg.custom.tipSin > 6 || cfg.custom.tipFse > 4) return false;
    BaseConfig base;
    memcpy_P(&base, &baseSinais[cfg.custom.tipSin - 1], sizeof(base));
    if (base.Ccf <= 0 && cfg.custom.tipFse > 0) return false;
    const int valores[] = {
      cfg.custom.dnt1, cfg.custom.lrd1, cfg.custom.dnt2, cfg.custom.lrd2,
      cfg.custom.dnt3, cfg.custom.lrd3, cfg.custom.dnt4, cfg.custom.lrd4
    };
    for (uint8_t i = 0; i < 8; ++i) {
      if (valores[i] < 0 || valores[i] > base.Ccf) return false;
    }
    const int inicios[] = { cfg.custom.dnt1, cfg.custom.dnt2, cfg.custom.dnt3, cfg.custom.dnt4 };
    const int fins[] = { cfg.custom.lrd1, cfg.custom.lrd2, cfg.custom.lrd3, cfg.custom.lrd4 };
    for (uint8_t i = 0; i < cfg.custom.tipFse; ++i) {
      if (fins[i] < inicios[i]) return false;
    }
  }
  return true;
}

static SinalConfig capturarSinalAtual() {
  SinalConfig atual;
  uint8_t sreg = SREG;
  cli();
  atual.tipSin = tipoSinalAtual;
  atual.tipFse = (byte)_faseOK;
  atual.dnt1 = _pI1D; atual.lrd1 = _l1D;
  atual.dnt2 = _pI2D; atual.lrd2 = _l2D;
  atual.dnt3 = _pI3D; atual.lrd3 = _l3D;
  atual.dnt4 = _pI4D; atual.lrd4 = _l4D;
  SREG = sreg;
  return atual;
}

static void aplicarSinal(const SinalConfig &sinal) {
  ConfigSinFase(sinal.tipSin, sinal.tipFse,
                sinal.dnt1, sinal.lrd1,
                sinal.dnt2, sinal.lrd2,
                sinal.dnt3, sinal.lrd3,
                sinal.dnt4, sinal.lrd4);
}

void inicializarAjustes() {
  EEPROM.get(EEPROM_ENDERECO, cfg);
  if (!configValida()) {
    valoresPadrao();
    inverterFonica = false;
    inverterFase = false;
    salvarConfig();
  } else {
    inverterFonica = (cfg.flags & 0x01) != 0;
    inverterFase = (cfg.flags & 0x02) != 0;
  }

  rpmSweepAtual = cfg.sweepMin;
  SelecionaSin(cfg.ultimoSinal);
  aplicarAjusteCustom(cfg.ultimoSinal);

  uint8_t sreg = SREG;
  cli();
  if (inverterFonica) PORTA |= (1 << 4); else PORTA &= ~(1 << 4);
  if (inverterFase)   PORTA |= (1 << 5); else PORTA &= ~(1 << 5);
  SREG = sreg;
}

void registrarSinalSelecionado(uint8_t sinal) {
  if (sinal == 0 || sinal > NUM_SINAIS) return;
  cfg.ultimoSinal = sinal;
  salvarConfig();
}

void aplicarAjusteCustom(uint8_t sinal) {
  if (cfg.customValido && cfg.sinalCustom == sinal) {
    aplicarSinal(cfg.custom);
  }
}

ModoRpm obterModoRpm() {
  return (ModoRpm)cfg.modoRpm;
}

uint16_t obterRpmFixo() {
  return cfg.rpmFixo;
}

float obterRpmControle(int adcPot) {
  if (cfg.modoRpm == RPM_FIXO) return (float)cfg.rpmFixo;

  if (cfg.modoRpm == RPM_VARREDURA) {
    unsigned long agora = millis();
    if (agora - ultimaEtapaSweep >= 100UL) {
      ultimaEtapaSweep = agora;
      if (sweepSubindo) {
        uint32_t proximo = (uint32_t)rpmSweepAtual + cfg.sweepPasso;
        if (proximo >= cfg.sweepMax) {
          rpmSweepAtual = cfg.sweepMax;
          sweepSubindo = false;
        } else {
          rpmSweepAtual = (uint16_t)proximo;
        }
      } else {
        if (rpmSweepAtual <= cfg.sweepMin + cfg.sweepPasso) {
          rpmSweepAtual = cfg.sweepMin;
          sweepSubindo = true;
        } else {
          rpmSweepAtual -= cfg.sweepPasso;
        }
      }
    }
    return (float)rpmSweepAtual;
  }

  return ((float)constrainADC(adcPot) / 1023.0f) * 5100.0f;
}

void acaoModoRpm(uint8_t parametro) {
  if (parametro == 0) {
    cfg.modoRpm = RPM_POTENCIOMETRO;
    salvarConfig();
    informarAjuste(F("Modo RPM"), F("Potenciometro A0"));
  } else if (parametro == 1) {
    cfg.modoRpm = RPM_FIXO;
    cfg.rpmFixo = 800;
    salvarConfig();
    informarAjusteNum(F("Modo RPM"), F("Fixo "), 800, F(" RPM"));
  } else if (parametro == 2) {
    cfg.modoRpm = RPM_FIXO;
    cfg.rpmFixo = 2000;
    salvarConfig();
    informarAjusteNum(F("Modo RPM"), F("Fixo "), 2000, F(" RPM"));
  } else if (parametro == 3) {
    cfg.modoRpm = RPM_FIXO;
    cfg.rpmFixo = 4000;
    salvarConfig();
    informarAjusteNum(F("Modo RPM"), F("Fixo "), 4000, F(" RPM"));
  } else {
    cfg.modoRpm = RPM_VARREDURA;
    rpmSweepAtual = cfg.sweepMin;
    sweepSubindo = true;
    salvarConfig();
    informarAjuste(F("Modo RPM"), F("Varredura 800-5000"));
  }
}

static int *campoEditor(uint8_t campo) {
  switch (campo) {
    case 1: return &editorCfg.dnt1;
    case 2: return &editorCfg.lrd1;
    case 3: return &editorCfg.dnt2;
    case 4: return &editorCfg.lrd2;
    case 5: return &editorCfg.dnt3;
    case 6: return &editorCfg.lrd3;
    case 7: return &editorCfg.dnt4;
    case 8: return &editorCfg.lrd4;
    default: return NULL;
  }
}

static void desenharEditor() {
  static const char *nomes[] = {
    "Qtd dentes", "F1 inicio", "F1 fim", "F2 inicio", "F2 fim",
    "F3 inicio", "F3 fim", "F4 inicio", "F4 fim"
  };

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EDITOR FASE S:");
  lcd.print((int)sinalAtual);
  lcd.setCursor(0, 1);
  lcd.print(nomes[editorCampo]);
  lcd.print(": ");
  if (editorCampo == 0) lcd.print((int)editorCfg.tipFse);
  else lcd.print(*campoEditor(editorCampo));
  lcd.setCursor(0, 2);
  lcd.print("Cima/Baixo ajusta");
  lcd.setCursor(0, 3);
  lcd.print("Enter prox Volta sai");
}

void acaoAbrirEditorFase(uint8_t) {
  if (sinalAtual == 0) SelecionaSin(cfg.ultimoSinal);
  if (_fSF <= 0) {
    informarAjuste(F("Editor Fase"), F("Sem ciclo editavel"), false);
    return;
  }
  editorBackup = capturarSinalAtual();
  editorCfg = editorBackup;
  if (editorCfg.tipFse == 0) editorCfg.tipFse = 1;
  editorCampo = 0;
  modoEditorFase = true;
  desenharEditor();
}

void editorFaseNavegar(int direcao) {
  if (!modoEditorFase) return;

  if (editorCampo == 0) {
    int valor = (int)editorCfg.tipFse + direcao;
    editorCfg.tipFse = (byte)constrain(valor, 0, 4);
  } else {
    int *valor = campoEditor(editorCampo);
    int limite = (_fSF > 0) ? _fSF : 240;
    *valor = constrain(*valor + direcao, 0, limite);
  }

  aplicarSinal(editorCfg);
  desenharEditor();
}

static void normalizarEditor() {
  int *inicios[] = { &editorCfg.dnt1, &editorCfg.dnt2, &editorCfg.dnt3, &editorCfg.dnt4 };
  int *fins[] = { &editorCfg.lrd1, &editorCfg.lrd2, &editorCfg.lrd3, &editorCfg.lrd4 };
  for (uint8_t i = 0; i < 4; ++i) {
    if (*fins[i] < *inicios[i]) *fins[i] = *inicios[i];
    if (i >= editorCfg.tipFse) {
      *inicios[i] = 0;
      *fins[i] = 0;
    }
  }
}

void editorFaseAvancar() {
  if (!modoEditorFase) return;

  uint8_t ultimoCampo = (uint8_t)(editorCfg.tipFse * 2);
  if (editorCampo < ultimoCampo) {
    ++editorCampo;
    desenharEditor();
    return;
  }

  normalizarEditor();
  aplicarSinal(editorCfg);
  cfg.custom = editorCfg;
  cfg.sinalCustom = sinalAtual;
  cfg.customValido = 1;
  salvarConfig();
  modoEditorFase = false;
  imprimirDumpFase();
  informarAjusteNum(F("Editor Fase"), F("Salvo sinal "), (int)sinalAtual, NULL);
  mostrarMenu(menuAtual, selecao, scrollMenu);
}

void editorFaseCancelar() {
  if (!modoEditorFase) return;
  aplicarSinal(editorBackup);
  modoEditorFase = false;
  informarAjuste(F("Editor Fase"), F("Cancelado"), false);
  mostrarMenu(menuAtual, selecao, scrollMenu);
}

void acaoInverterFonica(uint8_t) {
  uint8_t sreg = SREG;
  cli();
  inverterFonica = !inverterFonica;
  PORTA ^= (1 << 4);
  SREG = sreg;
  salvarConfig();
  informarAjuste(F("Inverter Fonica"),
                 inverterFonica ? F("Ativado") : F("Desativado"));
}

void acaoInverterFase(uint8_t) {
  uint8_t sreg = SREG;
  cli();
  inverterFase = !inverterFase;
  PORTA ^= (1 << 5);
  SREG = sreg;
  salvarConfig();
  informarAjuste(F("Inverter Fase"),
                 inverterFase ? F("Ativado") : F("Desativado"));
}

void acaoRestaurarFase(uint8_t) {
  if (!cfg.customValido || cfg.sinalCustom != sinalAtual) {
    informarAjuste(F("Restaurar Fase"), F("Sem custom ativa"), false);
    return;
  }
  cfg.customValido = 0;
  if (sinalAtual >= 1 && sinalAtual <= NUM_SINAIS) {
    SelecionaSin(sinalAtual);
  }
  salvarConfig();
  informarAjusteNum(F("Restaurar Fase"), F("Tabela sinal "), (int)sinalAtual, NULL);
}

void imprimirDumpFase() {
  SinalConfig atual = capturarSinalAtual();
  Serial.print(F("{ "));
  Serial.print(atual.tipSin); Serial.print(F(", "));
  Serial.print(atual.tipFse); Serial.print(F(", "));
  Serial.print(atual.dnt1); Serial.print(F(", "));
  Serial.print(atual.lrd1); Serial.print(F(", "));
  Serial.print(atual.dnt2); Serial.print(F(", "));
  Serial.print(atual.lrd2); Serial.print(F(", "));
  Serial.print(atual.dnt3); Serial.print(F(", "));
  Serial.print(atual.lrd3); Serial.print(F(", "));
  Serial.print(atual.dnt4); Serial.print(F(", "));
  Serial.print(atual.lrd4);
  Serial.println(F(" }"));
}

void imprimirStatus() {
  Serial.println(F("=== STATUS SIMULADOR ==="));
  Serial.print(F("Sinal: ")); Serial.println(sinalAtual);
  Serial.print(F("RPM modo: "));
  if (cfg.modoRpm == RPM_POTENCIOMETRO) Serial.println(F("POT"));
  else if (cfg.modoRpm == RPM_FIXO) Serial.println(F("FIXO"));
  else Serial.println(F("SWEEP"));
  Serial.print(F("RPM: ")); Serial.println((int)_rpm);
  Serial.print(F("RPM fixo: ")); Serial.println(cfg.rpmFixo);
  Serial.print(F("Sweep: ")); Serial.print(cfg.sweepMin);
  Serial.print('-'); Serial.println(cfg.sweepMax);
  Serial.print(F("Inverte fonica: ")); Serial.println(inverterFonica ? F("SIM") : F("NAO"));
  Serial.print(F("Inverte fase: ")); Serial.println(inverterFase ? F("SIM") : F("NAO"));
  printConfig();
}

static void aplicarFaseSerial(uint8_t dente, int inicio, int fim) {
  if (dente < 1 || dente > 4) return;
  if (_fSF <= 0) {
    Serial.println(F("ERRO: sinal atual nao possui ciclo de fase"));
    return;
  }
  SinalConfig atual = capturarSinalAtual();
  if (fim < inicio) fim = inicio;
  int limite = (_fSF > 0) ? _fSF : 240;
  inicio = constrain(inicio, 0, limite);
  fim = constrain(fim, 0, limite);
  if (atual.tipFse < dente) atual.tipFse = dente;

  int *inicios[] = { &atual.dnt1, &atual.dnt2, &atual.dnt3, &atual.dnt4 };
  int *fins[] = { &atual.lrd1, &atual.lrd2, &atual.lrd3, &atual.lrd4 };
  *inicios[dente - 1] = inicio;
  *fins[dente - 1] = fim;

  aplicarSinal(atual);
  cfg.custom = atual;
  cfg.sinalCustom = sinalAtual;
  cfg.customValido = 1;
  salvarConfig();
}

static void minusculas(char *texto) {
  for (; *texto; ++texto) *texto = (char)tolower((unsigned char)*texto);
}

static void executarComando(char *linha) {
  minusculas(linha);

  if (strcmp(linha, "status") == 0) {
    imprimirStatus();
  } else if (strcmp(linha, "dump") == 0) {
    imprimirDumpFase();
  } else if (strcmp(linha, "pot") == 0) {
    acaoModoRpm(0);
  } else if (strcmp(linha, "fixo") == 0) {
    cfg.modoRpm = RPM_FIXO;
    salvarConfig();
    informarAjusteNum(F("Modo RPM"), F("Fixo "), (int)cfg.rpmFixo, F(" RPM"));
  } else if (strcmp(linha, "sweep") == 0) {
    acaoModoRpm(4);
  } else if (strcmp(linha, "invertfonica") == 0) {
    acaoInverterFonica(0);
  } else if (strcmp(linha, "invertfase") == 0) {
    acaoInverterFase(0);
  } else if (strcmp(linha, "resetfase") == 0) {
    acaoRestaurarFase(0);
  } else if (strncmp(linha, "sinal=", 6) == 0) {
    int valor = atoi(linha + 6);
    if (valor >= 1 && valor <= NUM_SINAIS) {
      SelecionaSin((uint8_t)valor);
      aplicarAjusteCustom((uint8_t)valor);
      registrarSinalSelecionado((uint8_t)valor);
      _cP = 0;
      _cF = 0;
      if (_simulando) {
        _tempo = calcTempoTimer1(_rpm);
        Timer1_SetPeriod((unsigned long)_tempo);
      }
      informarAjusteNum(F("Sinal"), F("Selecionado "), valor, NULL);
    }
  } else if (strncmp(linha, "rpm=", 4) == 0) {
    int valor = atoi(linha + 4);
    if (valor >= 100 && valor <= 10000) {
      cfg.rpmFixo = (uint16_t)valor;
      cfg.modoRpm = RPM_FIXO;
      salvarConfig();
      informarAjusteNum(F("Modo RPM"), F("Fixo "), valor, F(" RPM"));
    }
  } else if (strncmp(linha, "fase", 4) == 0 && isdigit((unsigned char)linha[4]) && linha[5] == '=') {
    uint8_t dente = (uint8_t)(linha[4] - '0');
    char *virgula = strchr(linha + 6, ',');
    if (virgula) {
      *virgula = '\0';
      int inicio = atoi(linha + 6);
      int fim = atoi(virgula + 1);
      aplicarFaseSerial(dente, inicio, fim);
      informarAjusteNum(F("Fase Serial"), F("Dente "), (int)dente, NULL);
    }
  } else {
    Serial.println(F("Comandos: status dump pot fixo sweep rpm=N sinal=N faseN=ini,fim resetfase invertfonica invertfase"));
  }
}

void processarSerial() {
  static char linha[48];
  static uint8_t pos = 0;
  static bool descartando = false;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r' || c == '\n') {
      if (!descartando && pos > 0) {
        linha[pos] = '\0';
        executarComando(linha);
      }
      pos = 0;
      descartando = false;
    } else if (pos < sizeof(linha) - 1) {
      if (!descartando) linha[pos++] = c;
    } else {
      pos = 0;
      descartando = true;
    }
  }
}
