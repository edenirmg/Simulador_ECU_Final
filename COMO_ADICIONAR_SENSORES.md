# 🚗 Como Adicionar Novos Veículos e Sensores

Este guia explica passo a passo como expandir o simulador com novos modelos de veículos, novas marcas ou novos tipos de sensores.

---

## 📋 Índice

1. [Adicionar um novo veículo a uma marca existente](#1-adicionar-um-novo-veículo-a-uma-marca-existente)
2. [Adicionar uma nova marca de veículo](#2-adicionar-uma-nova-marca-de-veículo)
3. [Adicionar um novo tipo de roda fônica](#3-adicionar-um-novo-tipo-de-roda-fônica)
4. [Adicionar uma nova variável ao monitor analógico](#4-adicionar-uma-nova-variável-ao-monitor-analógico)
5. [Entender a tabela de sinais](#5-entender-a-tabela-de-sinais)

---

## 1. Adicionar um Novo Veículo a uma Marca Existente

### Exemplo: Adicionar "Fiat Toro 1.8" à marca Fiat

#### Passo 1 — Definir o sinal em `Sinais.cpp`

Adicione uma nova linha na tabela `sinais[]`:

```cpp
// Antes do último item (Fiat 500 1.4):
{ 5, 3, 28, 70, 88, 132, 150, 208, 0, 0 },  // 35: Fiat Toro 1.8
```

**Campos:**
- `5` → Tipo de roda fônica (60-2)
- `3` → 3 dentes de fase
- `28, 70` → Dente 1: início no pulso 28, termina no 70
- `88, 132` → Dente 2: início no pulso 88, termina no 132
- `150, 208` → Dente 3: início no pulso 150, termina no 208
- `0, 0` → Dente 4: não usado (tipFse = 3)

#### Passo 2 — Atualizar NUM_SINAIS em `Sinais.h`

```cpp
#define NUM_SINAIS  35   // Era 34, agora é 35
```

#### Passo 3 — Criar o texto do menu em `MenuData.cpp`

Na seção de strings Fiat:
```cpp
const char txtToro[] PROGMEM = "Toro 1.8 Flex";
```

#### Passo 4 — Declarar o texto em `MenuTextos.h`

Na seção Fiat:
```cpp
extern const char txtToro[] PROGMEM;
```

#### Passo 5 — Adicionar ao submenu em `MenuData.cpp`

No array `submenuFiat[]`, antes do `MENU_END`:
```cpp
MENU_ITEM(NULL, txtToro, &SelecionaSin, 35),  // 35 = índice na tabela
```

**Pronto!** O novo veículo aparecerá no menu Fiat automaticamente.

---

## 2. Adicionar uma Nova Marca de Veículo

### Exemplo: Adicionar "Hyundai" como nova marca

#### Passo 1 — Criar os sinais em `Sinais.cpp`

```cpp
// === Hyundai (índices 35-36) ===
{ 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },               // 35: Hyundai HB20 1.0
{ 5, 3, 30, 72, 90, 134, 152, 210, 0, 0 },      // 36: Hyundai Tucson 2.0
```

Atualize `NUM_SINAIS` em `Sinais.h` para 36.

#### Passo 2 — Criar strings PROGMEM em `MenuData.cpp`

```cpp
// --- Hyundai ---
const char tituloHyundai[] PROGMEM = "Hyundai";
const char txtHb20[] PROGMEM       = "HB20 1.0 12V";
const char txtTucson[] PROGMEM     = "Tucson 2.0 16V";
```

#### Passo 3 — Declarar em `MenuTextos.h`

```cpp
// HYUNDAI
extern const char tituloHyundai[] PROGMEM;
extern const char txtHb20[] PROGMEM;
extern const char txtTucson[] PROGMEM;
```

#### Passo 4 — Criar o submenu em `MenuData.cpp`

```cpp
const MenuItem submenuHyundai[] PROGMEM = {
  MENU_ITEM(NULL, txtHb20,    &SelecionaSin, 35),
  MENU_ITEM(NULL, txtTucson,  &SelecionaSin, 36),
  MENU_END
};
```

#### Passo 5 — Declarar em `MenuData.h`

```cpp
extern const MenuItem submenuHyundai[] PROGMEM;
```

#### Passo 6 — Adicionar ao menu de carros em `MenuData.cpp`

No array `submenuCarros[]`, antes do `MENU_END`:
```cpp
MENU_ITEM(submenuHyundai, txtHyundai, NULL, 0),
```

(Crie também `const char txtHyundai[] PROGMEM = "Hyundai";`)

#### Passo 7 — Registrar o título em `MenuData.cpp`

No array `titulosMenus[]`:
```cpp
{ submenuHyundai, tituloHyundai },
```

---

## 3. Adicionar um Novo Tipo de Roda Fônica

### Exemplo: Adicionar roda "48-2"

#### Passo 1 — Adicionar à tabela base em `Sinais.cpp`

```cpp
// No array baseSinais[]:
{ 7, 96, 100, 0, false, false, false, false, true, false, 200 }  // 48-2
```

**Campos:**
- `7` → Novo tipo (sequencial)
- `96` → 48×2 = 96 pulsos por volta
- `100` → Posição da falha
- `0` → Sem segunda falha
- Flags: não é Hall, não é 36, não é 44, não é 62
- `true` → É fônica padrão (1 falha)
- `false` → Não tem 2 falhas
- `200` → Comprimento do ciclo de fase

#### Passo 2 — Adicionar lógica em `Sensores.cpp` (se necessário)

A partir da **v2.7.1**, o período do Timer1 é calculado por `calcTempoTimer1()` em `Sensores.cpp`. Para tipos fônicos padrão (`_fonica` ou `_fonica1`), basta definir o campo **`falha`** em `baseSinais[]` — esse valor é usado como passos por volta no timer.

Se o novo tipo precisar de lógica diferente (ex.: flag nova em `BaseConfig`), estenda `calcTempoTimer1()`:

```cpp
// Em calcTempoTimer1() — exemplo para tipo com flag _fonica48
else if (_fonica48 && _falha > 0) {
  _rps = rpm / 60.0f;
  return calcPeriodo(_rps * (float)_falha, 1.0f);
}
```

> Não é mais necessário alterar `Simulador_ECU_Final.ino` para o cálculo de RPM — o `.ino` apenas chama `calcTempoTimer1(_rpm)`.

#### Passo 3 — Usar o novo tipo nos veículos

Em `Sinais.cpp`, nos sinais que usam 48-2:
```cpp
{ 7, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // Veículo com roda 48-2
```

---

## 4. Adicionar uma Nova Variável ao Monitor Analógico

### Exemplo: Adicionar leitura de "Knock Sensor (KS)"

#### Passo 1 — Atualizar Config.h

```cpp
#define MONITOR_VAR_COUNT  13   // Era 12, agora 13

const char* const MONITOR_LABELS[MONITOR_VAR_COUNT] = {
  "VSS:", "MAP:", "MAP/HZ:", "ACT:", "ECT:", "TPS:",
  "5_VOLTS(1):", "5_VOLTS(2):", "5_VOLTS(3):",
  "5_VOLTS(4):", "SONDA 1:", "SONDA 2:",
  "KS:"          // ← Nova variável
};
```

#### Passo 2 — Adicionar o cálculo em `Display.cpp`

Na função `atualizaLCD()`, na seção de cálculo de valores:
```cpp
valores[12] = _tensaoKs * 1.07f;  // Knock Sensor
```

#### Passo 3 — Adicionar o pino (se necessário) em `Config.h`

```cpp
#define PIN_POT_KS  A3  // Pino analógico para Knock Sensor
```

---

## 5. Entender a Tabela de Sinais

### Tipos de Roda Fônica (tipSin)

| Valor | Tipo | Dentes | Falhas | Passos/volta (`_falha`) |
|---|---|---|---|---|
| 1 | Hall contínuo | — | — | — (usa `calcPeriodo(rpm, 10)`) |
| 2 | 36-1 | 36 | 1 | 74 |
| 3 | 36-2 | 36 | 2 | 76 |
| 4 | 44-4 | 44 | 4 | 96 |
| 5 | 60-2 | 60 | 2 | 120 |
| 6 | 60-1+1 | 60 | 1+1 | 118 / 120 |

> O campo `_pulso` em `BaseConfig` define o padrão na ISR; `_falha` define os passos por volta usados em `calcTempoTimer1()` para sincronizar a frequência da ISR com o RPM.

### Configuração de Fase (tipFse)

- `0` → Sem sinal de fase
- `1` → 1 dente de referência (ex: Fiat Etorq)
- `2` → 2 dentes de referência
- `3` → 3 dentes de referência (ex: Fiat Fire, Firefly)
- `4` → 4 dentes de referência (ex: Audi TFSI, ME7.5)

### Diagrama de Fase

```
Ciclo de fase (ex: 60-2 com 3 dentes):

Pulsos: 0     dnt1  lrd1  dnt2  lrd2  dnt3  lrd3  _fSF(240)
        |      |     |     |     |     |     |     |
Fase:   LOW    HIGH──┘LOW  HIGH──┘LOW  HIGH──┘LOW  (reset)
```

### Checklist para Novo Veículo

- [ ] Identificar tipo de roda fônica do veículo
- [ ] Levantar diagrama de fase (osciloscópio ou manual)
- [ ] Anotar posições dos dentes de fase em número de pulsos
- [ ] Adicionar entrada em `sinais[]` (Sinais.cpp)
- [ ] Atualizar `NUM_SINAIS` (Sinais.h)
- [ ] Criar string PROGMEM (MenuData.cpp)
- [ ] Declarar string (MenuTextos.h)
- [ ] Adicionar ao submenu da marca (MenuData.cpp)
- [ ] Compilar e testar no Arduino IDE

---

## ⚠️ Dicas Importantes

1. **Índices são 1-based**: O primeiro sinal na tabela é o índice 1, não 0
2. **PROGMEM**: Todas as tabelas e strings ficam na Flash (não na RAM)
3. **Tamanho do texto**: O LCD tem 20 colunas; textos devem ter no máximo 19 caracteres (1 coluna para o cursor ">")
4. **Teste incremental**: Adicione um veículo por vez e compile para verificar erros
5. **Serial Monitor**: Use `printConfig()` para debugar a configuração carregada
