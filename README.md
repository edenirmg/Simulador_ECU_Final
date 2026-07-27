# 🔧 Simulador de ECU — Arduino Mega 2560

**Versão 2.8.0** | Autor: Edemir Marques Gelocha

Simulador de ECU (Electronic Control Unit) completo para Arduino Mega 2560. Gera sinais de roda fônica, fase, VSS e MAP PWM por hardware timer para testes e diagnósticos automotivos em bancada.

---

## ✨ Funcionalidades

### Sinais de Roda Fônica
- **Hall contínuo** — sinal quadrado simples (sensor Hall)
- **36-1** / **36-2** — rodas de 36 dentes com 1 ou 2 falhas
- **44-4** — roda de 44 dentes com 4 falhas (Renault/Nissan)
- **60-2** / **60-1+1** — rodas de 60 dentes (GM, VW, Fiat, etc.)
- **Fase configurável** — 1 a 4 dentes de fase com posição e largura ajustáveis

### 83 Sinais Pré-Configurados
Gasolina + **Diesel Leve** + **Diesel Pesado** (PLD, EMS S6, MS6.x, EDC7, D12/D13…). Fase calibrada onde disponível; stubs usam roda correta sem fase.

### Sinais de Saída por Hardware Timer
- **VSS** (Vehicle Speed Sensor) — Timer3 CTC, pino 5 (OC3A), 0-860 Hz
- **MAP PWM** (Manifold Absolute Pressure) — Timer5 CTC, pino 46 (OC5A), 95-159 Hz

### Monitor Analógico em Tempo Real
- RPM, VSS, MAP, ACT, ECT, TPS, 5V sensores (×4), Sondas Lambda (×2)
- Navegação por scroll com display LCD 20x4

### Interface de Menu
- Menu hierárquico com scroll automático
- Navegação por 4 botões (Subir, Descer, Voltar, Enter)
- Seleção de veículos por marca e modelo
- Menu **Ajustes**: RPM por potenciômetro, fixo ou varredura; editor de fase; inversão lógica

### Configuração e Automação
- EEPROM interna do ATmega2560 com assinatura, versão e checksum
- Restaura último sinal, modo RPM, fase customizada e polaridades ao ligar
- Controle Serial sem bloqueio: `sinal=`, `rpm=`, `faseN=`, `status` e `dump`

### Controle de Relés
- Seleção Hall Magnético / Indutivo
- Tensão de saída 5V / 12V
- Fase Hall / Magnético

---

## 🛠️ Hardware Necessário

| Componente | Quantidade | Pinos |
|------------|-----------|-------|
| Arduino Mega 2560 | 1 | — |
| Display LCD 20×4 I2C | 1 | SDA/SCL (I2C, endereço 0x20) |
| Botões de navegação | 4 | 22 (Descer), 23 (Enter), 24 (Subir), 25 (Voltar) |
| Relés | 3 | 50, 51, 52 |
| Potenciômetros 10kΩ | 3 | A0 (RPM), A1 (VSS), A10 (MAP) |
| Divisores resistivos 5:1 | 8 | A2-A9 (entradas 0-25V) |
| Sondas lambda (0-5V direto) | 2 | A11 (Sonda 1), A12 (Sonda 2) |

### Saídas de Sinal

| Pino | Sinal | Timer | Descrição |
|------|-------|-------|-----------|
| 26 (PA4) | Fônica/Hall | **Timer1** (ISR) | Um “passo” do dente a cada interrupção — ver [Timer1](#-timer1-sinal-fônica-hall-e-fase) |
| 27 (PA5) | Fase | **Timer1** (ISR) | Sinal de fase sincronizado ao mesmo período |
| 5 (OC3A) | VSS | Timer3 (CTC) | Velocidade do veículo |
| 46 (OC5A) | MAP PWM | Timer5 (CTC) | Pressão do coletor |

---

## 📦 Instalação

### Bibliotecas Necessárias

Instale via **Arduino IDE → Ferramentas → Gerenciador de Bibliotecas**:

| Biblioteca | Versão | Uso |
|-----------|--------|-----|
| `LiquidCrystal_I2C` | ≥ 1.1.2 | Display LCD I2C |
| `TimerFour` | ≥ 1.0.0 | Timer4 — leitura periódica de sensores (20 Hz) |
| `TimerFive` | ≥ 1.0.0 | Instalada; MAP usa registradores diretos no código |

> **Timer1 (fônica/Hall/fase)** não usa biblioteca externa: controle em `Timer1_Direct.cpp` + `ISR_Timer1.cpp`. Não instale `TimerOne` junto com este projeto (conflito na mesma ISR).

> **Nota:** `Wire.h` e `avr/pgmspace.h` já vêm incluídas no Arduino IDE.

### Compilação e Upload

1. Abra `Simulador_ECU_Final.ino` na Arduino IDE
2. Selecione: **Ferramentas → Placa → Arduino Mega or Mega 2560**
3. Selecione a porta serial correta
4. Clique em **Upload** (Ctrl+U)

---

## 📂 Estrutura do Projeto

```
Simulador_ECU_Final/
├── Simulador_ECU_Final.ino   ← Arquivo principal (setup + loop)
├── Config.h                  ← Constantes, pinos e calibração
├── Tipos.h                   ← Structs (MenuItem, SinalConfig, BaseConfig)
├── Sinais.h / Sinais.cpp     ← Tabela de 83 sinais + configuração de roda/fase
├── Ajustes.h / Ajustes.cpp   ← EEPROM, modos RPM, editor de fase e comandos Serial
├── MenuTextos.h              ← Strings PROGMEM dos menus
├── MenuData.h / MenuData.cpp ← Estrutura hierárquica dos menus
├── MenuNav.h / MenuNav.cpp   ← Navegação (botões, scroll, pilha)
├── Display.h / Display.cpp   ← Controle do LCD 20x4 (menus + monitor)
├── Sensores.h / Sensores.cpp ← Leitura analógica, Timer3 VSS, Timer5 MAP, calcTempoTimer1, lógica da ISR
├── Timer1_Direct.h / .cpp    ← Configuração do Timer1 (CTC, OCR1A)
├── ISR_Timer1.cpp            ← Vetor TIMER1_COMPA_vect (fônica + fase)
├── Reles.h / Reles.cpp       ← Controle dos relés de saída
│
├── README.md                 ← Este arquivo
├── CHANGELOG.md              ← Histórico de alterações
├── ARQUITETURA.md            ← Explicação da estrutura modular
├── GUIA_RAPIDO.md            ← Guia rápido de uso
├── COMO_ADICIONAR_SENSORES.md← Como expandir com novos veículos/sensores
│
└── Testes/
    └── Timer5_Pino46/
        └── Teste_Timer5_Pino46.ino  ← Teste isolado do Timer5
```

> Veja [ARQUITETURA.md](ARQUITETURA.md) para a explicação detalhada de cada módulo.

---

## ⚙️ Timer1 — Sinal Fônica, Hall e Fase

O **Timer1** do ATmega2560 define **com que frequência** o simulador avança o padrão da roda fônica (pino 26) e da fase (pino 27). A cada interrupção, o firmware incrementa contadores internos e altera os pinos de `PORTA` — não há PWM automático nesses pinos.

### Resumo

| Conceito | Valor / comportamento |
|----------|------------------------|
| Modo | CTC, TOP = `OCR1A`, prescaler **64** |
| Clock efetivo | 250 kHz → **4 µs** por tick |
| Período ISR | `T ≈ 4 × (OCR1A + 1)` microssegundos |
| RPM | Potenciômetro **A0**, valor fixo ou varredura |
| Ajuste no loop | `Timer1_SetPeriod(_tempo)` no máximo a cada **50 ms** se o RPM mudar **> 1%** |

### Fluxo simplificado

1. **Menu** — escolhe veículo → `SelecionaSin()` carrega tipo de roda e `_falha` da tabela `baseSinais[]`.
2. **Início da simulação** — `AtivaMonitorAnalogico()` obtém o RPM do modo ativo, calcula `calcTempoTimer1(rpm)` e só então chama `Timer1_SetPeriod()` + `Timer1_Start()`.
3. **Loop** — `obterRpmControle()` escolhe potenciômetro, valor fixo ou varredura; depois atualiza o Timer1 (throttle 50 ms, ΔRPM > 1%).
4. **ISR** — `ISR_Timer1.cpp` → `InterruptTimer1()` gera os níveis nos pinos 26 e 27.

### Fórmula do período (exemplos)

A função central é `calcTempoTimer1()` em `Sensores.cpp`, que usa `calcPeriodo(freqHz, fator)`:

`periodo_us = (fator / freqHz) × 1.000.000` — limitado entre `TIMER1_MIN_US` e `TIMER1_MAX_US`.

- **Hall:** `calcPeriodo(rpm, 10.0f)`.
- **Fônica (36-1, 44-4, 60-2, …):** `_rps = rpm/60`, `calcPeriodo(_rps × _falha, 1.0f)` — `_falha` vem de `baseSinais[]` (ex.: 74 para 36-1, 96 para 44-4, 120 para 60-2).

Limites em `Config.h`: `TIMER1_MIN_US` (50 µs) e `TIMER1_MAX_US` (262 ms).

Documentação completa (registradores, tabelas, diagrama de sequência, boas práticas): **[ARQUITETURA.md → Timer1](ARQUITETURA.md#timer1--sinal-fônica-hall-e-fase)**.

---

## 🚀 Uso Rápido

1. Conecte o hardware conforme a tabela acima
2. Faça upload do sketch
3. No LCD, navegue com os botões:
   - **Subir/Descer** — navegar no menu
   - **Enter** — selecionar veículo ou submenu
   - **Voltar** — retornar ao menu anterior (para simulação)
4. Ao selecionar um veículo, o simulador:
   - Configura o sinal de fônica/fase
   - Ativa os timers VSS e MAP
   - Exibe o monitor analógico em tempo real
5. Gire os potenciômetros para ajustar RPM, VSS e MAP

> Veja [GUIA_RAPIDO.md](GUIA_RAPIDO.md) para instruções detalhadas.

---

## ⌨️ Controle Serial (9600 baud)

- `status` / `dump` — estado completo e fase atual
- `sinal=12` — seleciona uma entrada de `sinais[]` pelo índice 1-based
- `rpm=2500`, `pot`, `fixo`, `sweep` — controle do modo RPM
- `fase1=32,72` até `fase4=...` — início/fim dos dentes de fase
- `resetfase` — restaura a fase original da tabela
- `invertfonica` / `invertfase` — alterna a polaridade lógica

Alterações persistentes usam `EEPROM.put()`, que só regrava bytes modificados.

---

## 🧪 Testes

O projeto inclui um sketch de teste isolado para diagnóstico do Timer5 (MAP PWM).

> ⚠️ O teste fica em `Testes/Timer5_Pino46/` e deve ser compilado **separadamente** — não junto com o projeto principal. Veja instruções em [GUIA_RAPIDO.md](GUIA_RAPIDO.md#teste-isolado-timer5).

---

## 📜 Histórico de Versões

| Versão | Principais Alterações |
|--------|----------------------|
| **2.8.0** | EEPROM interna, editor de fase, RPM fixo/varredura, parser Serial e inversão lógica |
| **2.7.2** | Correção dos índices do menu e seleção de veículos |
| **2.7.1** | Correção RPM/Timer1 (`calcPeriodo`, `_falha`, `TIMER1_MAX_US` 262 ms); período correto ao iniciar simulação |
| **2.7** | Timer1 por registradores (`Timer1_Direct` + `ISR_Timer1`); documentação ampliada do Timer1 |
| **2.6** | Correção causa raiz: loop rápido impedia Timer3/Timer5 de gerar sinal (detect-if-running) |
| **2.5** | Correção flags VSS/MAP, Timer5 OC5B→OC5A, seleção direta de sinal |
| **2.3** | Divisor resistivo 5:1, conversão ADC para 0-25V |
| **2.2** | Proteções NaN/Inf, ISR Timer4 não-bloqueante, tabela NTC expandida |

> Veja [CHANGELOG.md](CHANGELOG.md) para o histórico completo.

---

## 🔧 Personalização

### Alterar endereço I2C do LCD
Em `Config.h`, modifique `LCD_ENDERECO`:
```cpp
#define LCD_ENDERECO  0x27  // Endereço I2C do seu display
```

### Alterar pinos dos botões
Em `Config.h`, modifique o array `BOTOES_PINOS[]`:
```cpp
const int BOTOES_PINOS[] = { 24, 22, 25, 23 };  // Subir, Descer, Voltar, Enter
```

### Calibrar fator VSS
Em `Config.h`, ajuste `FATOR_VSS`:
```cpp
#define FATOR_VSS  4.3f  // Fator km/h → Hz (varia conforme veículo)
```

### Adicionar novos veículos
Veja [COMO_ADICIONAR_SENSORES.md](COMO_ADICIONAR_SENSORES.md) para instruções passo a passo.
