# 🏗️ Arquitetura do Projeto — Simulador de ECU v2.8.0

## Visão Geral

O projeto é organizado em **módulos** (pares `.h`/`.cpp`) mais o arquivo principal `.ino`. Cada módulo tem responsabilidade única e comunica-se com os demais via variáveis globais `extern` e chamadas de função.

```
┌──────────────────────────────────────────────────┐
│           Simulador_ECU_Final.ino                │
│           (setup + loop principal)               │
├─────────┬─────────┬──────────┬──────────┬────────┤
│ MenuNav │ Display │ Sensores │  Sinais  │ Reles  │
├─────────┴─────────┴──────────┴──────────┴────────┤
│ Timer1_Direct.cpp  │  ISR_Timer1.cpp  (Timer1)    │
├───────────────────┴──────────────────────────────┤
│ MenuData │ MenuTextos │ Tipos │ Config            │
└──────────────────────────────────────────────────┘
```

> Documentação aprofundada do **Timer1** (fônica, Hall e fase): [seção dedicada abaixo](#timer1--sinal-fônica-hall-e-fase).

---

## Módulos

### 1. `Config.h` — Configurações Centralizadas
**Responsabilidade:** Concentra TODAS as constantes, pinos e parâmetros de calibração.

- Endereço I2C do LCD, dimensões
- Pinos dos botões, potenciômetros, relés
- Pinos de saída de sinal (Timer3, Timer5)
- Constantes de calibração (F_CPU_HZ, PRESCALER, FATOR_VSS)
- Períodos dos timers (Timer1, Timer4)
- Funções inline de validação (constrainADC, constrainFreq, etc.)

**Depende de:** `Arduino.h`

> 💡 Para trocar hardware (pinos, endereço LCD), edite **apenas** este arquivo.

---

### 2. `Tipos.h` — Definições de Structs
**Responsabilidade:** Define as estruturas de dados usadas em todo o projeto.

| Struct | Uso |
|--------|-----|
| `MenuItem` | Item de menu (texto PROGMEM, submenu, ação, parâmetro) |
| `MenuTitulo` | Associa um array de MenuItems ao seu título |
| `SinalConfig` | Configuração de sinal por veículo (tipo base + fase) |
| `BaseConfig` | Propriedades de cada tipo de roda fônica |

**Macros:** `MENU_ITEM()` e `MENU_END` simplificam declaração de menus.

**Depende de:** `Arduino.h`

---

### 3. `Sinais.h` / `Sinais.cpp` — Tabelas de Sinais
**Responsabilidade:** Tabelas PROGMEM com 55 configurações de sinal e tipos de roda fônica.

- `sinais[]` — Array PROGMEM com `SinalConfig` para cada veículo
- `baseSinais[]` — Array PROGMEM com `BaseConfig` para cada tipo de roda
- `SelecionaSin(idx)` — Carrega configuração do veículo selecionado
- `ConfigSinFase(...)` — Aplica parâmetros de sinal e fase

**Variáveis globais:** `_pulso`, `_falha`, `_hall`, `_fonica`, `_simulando`, `_cP`, `_cF`, etc.

**Depende de:** `Tipos.h`

---

### 3.1. `Ajustes.h` / `Ajustes.cpp` — Configuração Persistente
**Responsabilidade:** reúne as configurações mutáveis sem alterar as tabelas PROGMEM.

- EEPROM interna com magic, versão e checksum
- Modos RPM: potenciômetro, fixo e varredura
- Editor LCD da fase customizada
- Parser Serial não bloqueante
- Polaridade lógica independente para fônica e fase

As escritas da fase são aplicadas atomicamente em `ConfigSinFase()`. A EEPROM só é atualizada ao concluir uma edição ou alterar uma configuração, nunca a cada passo da varredura.

---

### 4. `MenuTextos.h` — Strings PROGMEM
**Responsabilidade:** Centraliza todas as strings de texto dos menus em Flash (PROGMEM).

- Títulos de menus (`tituloPrincipal`, `tituloAudi`, etc.)
- Textos de itens (`txtCarros`, `txtHall`, `txtAudiTfsi`, etc.)

> Armazenar strings em PROGMEM economiza ~2KB de RAM.

**Depende de:** `avr/pgmspace.h`

---

### 5. `MenuData.h` / `MenuData.cpp` — Estrutura dos Menus
**Responsabilidade:** Define a hierarquia completa dos menus como arrays PROGMEM de `MenuItem`.

- `menuPrincipal[]` — Menu raiz
- `submenuCarros[]`, `submenuAudi[]`, `submenuBMW[]`, ... — Submenus por marca
- `submenuHallMagnetico[]`, `submenuHall5x12[]` — Configuração de relés
- `titulosMenus[]` — Tabela que associa cada array ao seu título
- `contarItens()`, `readMenuItem()`, `readMenuItemTexto()` — Utilidades PROGMEM

**Depende de:** `Tipos.h`, `MenuTextos.h`, `Sinais.h`, `Reles.h`

---

### 6. `MenuNav.h` / `MenuNav.cpp` — Navegação
**Responsabilidade:** Lógica de interação do usuário com os menus.

| Função | Descrição |
|--------|-----------|
| `botaoPressionadoIndice()` | Detecta botão com debounce (250ms) |
| `navegar(dir)` | Move cursor no menu ou scroll no monitor |
| `entrar()` | Entra em submenu ou executa ação |
| `voltar()` | Volta ao menu anterior (pilha) |
| `AtivaMonitorAnalogico(param)` | Inicia simulação e ativa monitor |

**Estado:** `menuAtual`, `selecao`, `scrollMenu`, pilha de menus.

**Depende de:** `Config.h`, `Display.h`, `MenuData.h`, `Sinais.h`, `Sensores.h`, `Reles.h`, `Timer1_Direct.h`, `TimerFour.h`

---

### 7. `Display.h` / `Display.cpp` — Controle do LCD
**Responsabilidade:** Toda comunicação com o display LCD 20×4 I2C.

| Função | Descrição |
|--------|-----------|
| `inicializarDisplay()` | Configura LCD (backlight, clear) |
| `mostrarMenu(menu, sel, scroll)` | Renderiza menu com cursor e scroll |
| `desenhaTelaMonitor()` | Desenha layout fixo do monitor analógico |
| `atualizaLCD()` | Atualiza valores dinâmicos (smart-update) |

**Smart-update:** Compara buffer atual com anterior byte-a-byte. Só reescreve posições que mudaram, eliminando flicker e reduzindo tempo de I2C.

**Depende de:** `Config.h`, `Sinais.h`, `Sensores.h`, `MenuData.h`, `LiquidCrystal_I2C.h`

---

### 8. `Sensores.h` / `Sensores.cpp` — Sensores e Timers de Hardware
**Responsabilidade:** O módulo mais crítico — controla todos os timers e leituras analógicas.

#### Leitura de Sensores
- `leSensoresAnalogicos()` — Lê 10 pinos analógicos, mapeia para variáveis de tensão
- `lePorta()` — Leitura com média (12 amostras, ~60ms) — uso em calibração

#### Conversões
- `calcTempoTimer1()` — Período Timer1 a partir do RPM e do sinal selecionado (`_hall`, `_falha`)
- `calcPeriodo()` — Converte frequência (Hz) em período (µs) para Timer1
- `ntcInterpolate()` — Tabela NTC (19 pontos, -40°C a 150°C)
- `converteACT()`, `converteECT()`, `converteTPS()`, `converteMAP()`

#### Timer3 — VSS
- `setupTimer3_VSS(freq)` — Configura CTC no OC3A (pino 5)
- `stopTimer3()` — Para timer e pino em LOW
- `setVssKmh(kmh)` — Interface de alto nível

#### Timer5 — MAP PWM
- `setVacuumFreq(freq)` — Configura CTC no OC5A (pino 46)
- `stopTimer5()` — Para timer e pino em LOW

#### ISRs
- `InterruptTimer1()` — Lógica do sinal fônica/hall/fase (PORTA bits 4-5); chamada por `ISR_Timer1.cpp`
- `InterruptTimer4()` — Seta flag `readSensors` (20 Hz)

> ⚠️ **Padrão detect-if-running:** `setupTimer3_VSS()` e `setVacuumFreq()` verificam se o timer já está rodando antes de reconfigurar. Se sim, só atualizam OCR quando o valor muda. Isso evita que o loop rápido (~400µs) resete TCNT antes do compare match.

**Depende de:** `Config.h`, `Sinais.h`

---

### 9. `Timer1_Direct.h` / `Timer1_Direct.cpp` + `ISR_Timer1.cpp` — Timer1

| Arquivo | Função |
|---------|--------|
| `Timer1_Direct.cpp` | Configura registradores (`TCCR1A/B`, `OCR1A`, `TIMSK1`) |
| `ISR_Timer1.cpp` | Vetor `TIMER1_COMPA_vect` (única definição da ISR) |
| `Sensores.cpp` | `InterruptTimer1()` — padrão de dentes em `PORTA` |

**Depende de:** `Arduino.h`, `avr/interrupt.h` | Ver também: [Timer1 — Sinal Fônica, Hall e Fase](#timer1--sinal-fônica-hall-e-fase)

---

### 10. `Reles.h` / `Reles.cpp` — Controle de Relés
**Responsabilidade:** Interface com os 3 relés de saída.

| Função | Descrição |
|--------|-----------|
| `inicializarReles()` | Configura pinos como OUTPUT, estado LOW |
| `releOff()` | Desliga todos os relés |
| `releHallMagnetico(valor)` | Seleciona tipo Hall/Magnético |
| `releHall5x12(valor)` | Seleciona tensão 5V/12V |

**Depende de:** `Config.h`, `MenuNav.h`

---

## Fluxo de Execução

### Setup
```
setup()
  ├── Serial.begin(9600)
  ├── inicializarDisplay()        → LCD pronto
  ├── pinMode(BOTOES[], PULLUP)   → Botões prontos
  ├── inicializarReles()          → Relés desligados
  ├── DDRA |= 0xF0                → Pinos 26-29 como saída
  ├── Timer1_Initialize()         → CTC + OCR1A seguro; timer PARADO
  ├── Timer4.initialize()         → Configurado, PARADO
  └── mostrarMenu()               → Menu principal no LCD
```

### Loop (sem simulação)
```
loop()
  ├── botaoPressionadoIndice()    → Detecta botão
  │   ├── navegar(-1)             → Sobe no menu
  │   ├── navegar(+1)             → Desce no menu
  │   ├── voltar()                → Volta (para simulação se ativa)
  │   └── entrar()                → Entra submenu / executa ação
  │       └── AtivaMonitorAnalogico()
  │           ├── SelecionaSin()       → Configura sinal (_pulso, _falha, _faseOK, …)
  │           ├── Lê A0 → calcTempoTimer1() → período correto
  │           ├── Timer1_SetPeriod()   → Aplica período calculado
  │           ├── Timer1_Start()       → Habilita contagem Timer1
  │           ├── Timer4.start()       → Leitura de sensores (20 Hz)
  │           └── vssAtivo=true, mapHzAtivo=true
  └── (nada mais — aguarda botão)
```

### Loop (simulação ativa)
```
loop()
  ├── botaoPressionadoIndice()    → Navega no monitor analógico
  │
  ├── if (readSensors)            → Flag do Timer4 ISR (20 Hz)
  │   └── leSensoresAnalogicos()  → Lê A2-A13, seta newData
  │
  └── if (_simulando)
      ├── analogRead(A0, A1, A10) → Lê potenciômetros (~336µs)
      ├── Calcula RPM             → A0 → 0-5100 RPM
      ├── _tempo = calcTempoTimer1(_rpm) → Período conforme Hall/fônica/_falha
      │
      ├── if (vssAtivo)
      │   └── setVssKmh(kmh)      → Atualiza Timer3 (só OCR)
      │
      ├── if (mapHzAtivo)
      │   └── setVacuumFreq(freq) → Atualiza Timer5 (só OCR)
      │
      ├── (a cada 50 ms, se RPM mudou >1%) Timer1_SetPeriod(_tempo)
      └── atualizaLCD()           → Smart-update no LCD
```

### ISRs (executam independentemente)
```
Timer1 ISR — período = _tempo µs (50 µs … 262 ms, conforme RPM)
  ISR(TIMER1_COMPA_vect)          → ISR_Timer1.cpp
  └── InterruptTimer1()           → Sensores.cpp
      ├── _cP++ / padrão Hall ou fônica → PORTA bit4 (pino 26)
      └── _cF++ / dentes de fase        → PORTA bit5 (pino 27)

Timer4 ISR (50ms = 20 Hz)
  └── InterruptTimer4()
      └── readSensors = true    → Flag para o loop()

Timer3 (hardware CTC, sem ISR)
  └── OC3A toggle automático    → Pino 5, sinal VSS

Timer5 (hardware CTC, sem ISR)
  └── OC5A toggle automático    → Pino 46, sinal MAP
```

---

## Timer1 — Sinal Fônica, Hall e Fase

O **Timer1** é o único timer usado com **interrupção por software** para o sinal principal da ECU. Os timers 3 e 5 geram VSS e MAP em hardware (toggle no pino); o Timer4 só sinaliza leitura de ADC. O Timer1 dispara a cada período configurável e, na ISR, avança um “passo” do dente virtual da roda fônica e do sinal de fase.

### Papel no sistema

| Item | Detalhe |
|------|---------|
| **Saídas** | Pino **26** (PA4) = fônica/Hall; pino **27** (PA5) = fase |
| **Entrada de RPM** | Potenciômetro em **A0** → `_rpm` no `loop()` |
| **Período** | Variável global `_tempo` (µs) → `OCR1A` via `Timer1_SetPeriod()` |
| **Padrão do sinal** | Tabelas em `Sinais.cpp` (`_pulso`, `_falha`, `_faseOK`, `_pI1D`…) |
| **Contadores ISR** | `_cP` (pulsos fônica), `_cF` (ciclo de fase) |

### Por que controle direto (sem biblioteca TimerOne)?

1. **Uma implementação só** — registradores em `Timer1_Direct.cpp`; a ISR em `ISR_Timer1.cpp`. Evita conflito de símbolos com TimerOne e com múltiplos `#include` de um `.h` com funções.
2. **Período em microssegundos** — o loop calcula `_tempo` com `calcPeriodo()` e passa o mesmo valor para `Timer1_SetPeriod()`.
3. **CTC explícito** — modo Compare Match com TOP = `OCR1A` e prescaler fixo 64, igual à documentação do ATmega2560.

### Configuração de hardware (ATmega2560)

```
F_CPU = 16 MHz
Prescaler Timer1 = 64  →  frequência do contador = 250 kHz  →  1 tick = 4 µs

Modo CTC (WGM12 = 1): quando TCNT1 == OCR1A, TCNT1 ← 0 e dispara TIMER1_COMPA_vect

Período da interrupção:
  T_isr [µs] = 4 × (OCR1A + 1)

Timer1_SetPeriod(periodo_us):
  OCR1A = (periodo_us / 4) - 1   (limitado a 0 … 65535)
```

**Faixa teórica do timer:** ~4 µs a ~262 ms por interrupção. No projeto, `Config.h` restringe o que o loop envia:

| Constante | Valor | Efeito |
|-----------|-------|--------|
| `TIMER1_MIN_US` | 50 µs | Evita período zero ou RPM absurdamente alto |
| `TIMER1_MAX_US` | 262.000 µs | Máximo do OCR1A (65535×4 µs) — RPM baixo real |
| `TIMER1_PERIODO_US` | 10.000 µs | Fallback se o tipo de sinal não for Hall nem fônica |

### Do RPM ao período (no `loop()`)

O potenciômetro A0 mapeia 0–1023 → **0–5100 RPM** (`_rpm`). O cálculo está centralizado em `calcTempoTimer1()` (`Sensores.cpp`), chamado no `loop()` e em `AtivaMonitorAnalogico()`:

| Tipo | Condição | Cálculo de `_tempo` |
|------|----------|---------------------|
| Parado | `rpm < 0,5` | `TIMER1_MAX_US` |
| Hall contínuo | `_hall` | `calcPeriodo(rpm, 10.0f)` |
| Fônica | `_fonica \|\| _fonica1` e `_falha > 0` | `calcPeriodo(_rps × _falha, 1.0f)` |
| Outros | — | `TIMER1_PERIODO_US` |

`_rps = rpm / 60`. O campo `_falha` vem de `baseSinais[]` via `SelecionaSin()` — passos por volta para o timer (74 para 36-1, 96 para 44-4, 120 para 60-2, etc.).

A função `calcPeriodo(freqHz, fator)` em `Sensores.h` implementa:

`periodo_us = (fator / freqHz) × 1.000.000` — limitado internamente a `TIMER1_MIN_US` … `TIMER1_MAX_US`.

### O que acontece dentro da ISR

A cada compare match, `InterruptTimer1()` (em `Sensores.cpp`):

1. Incrementa `_cP` e gera o nível em **PA4**:
   - **Hall:** alterna com `PORTA ^= (1<<4)` a cada 2 contagens.
   - **Fônica:** alterna até `_pulso`, mantém nível na “falha” (`_falha`, `_falha1`), reinicia o ciclo.
2. Incrementa `_cF` e, conforme `_faseOK` (0–4), define **PA5** com base em `_pI1D`/`_l1D` … `_pI4D`/`_l4D`.
3. Zera `_cF` quando atinge `_fSF` (comprimento do ciclo de fase).

A ISR **não** chama `analogRead`, `Serial` nem LCD — isso manteria o Timer1 bloqueado por milissegundos.

### Ciclo de vida no firmware

```mermaid
sequenceDiagram
  participant Setup
  participant Menu as MenuNav
  participant Loop
  participant T1 as Timer1 HW
  participant ISR as ISR_Timer1

  Setup->>T1: Timer1_Initialize()
  Menu->>Menu: Lê A0 → calcTempoTimer1()
  Menu->>T1: Timer1_SetPeriod + Timer1_Start (simulação ON)
  Loop->>Loop: Lê A0, calcTempoTimer1(_rpm)
  Loop->>T1: Timer1_SetPeriod (máx. a cada 50 ms, ΔRPM>1%)
  T1->>ISR: TIMER1_COMPA_vect
  ISR->>ISR: InterruptTimer1() → PORTA
  Menu->>T1: Timer1_Stop (Voltar / sair)
```

| Momento | Função | Observação |
|---------|--------|------------|
| `setup()` | `Timer1_Initialize()` | Configura CTC e OCR1A; **não** inicia contagem |
| `AtivaMonitorAnalogico()` | Zera `_cP`/`_cF`; lê A0 → `calcTempoTimer1()` → `Stop` → `SetPeriod` → `Start` | Período correto antes de iniciar |
| `loop()` com `_simulando` | `Timer1_SetPeriod(_tempo)` | Throttle: intervalo 50 ms e mudança > 1% no RPM |
| `voltar()` | `Timer1_Stop()` | Para fônica/fase junto com Timer4, Timer3/5 |

### Arquivos e responsabilidades

```
Timer1_Direct.h     → declarações (Timer1_Initialize, SetPeriod, Stop, Start)
Timer1_Direct.cpp   → escrita em TCCR1A/B, TCNT1, OCR1A, TIMSK1
ISR_Timer1.cpp      → ISR(TIMER1_COMPA_vect) { InterruptTimer1(); }
Sensores.cpp        → calcTempoTimer1(), calcPeriodo(), InterruptTimer1()
Sinais.cpp          → _pulso, _hall, _faseOK, … (lidos na ISR, escritos no menu)
Simulador_ECU_Final.ino → chama calcTempoTimer1(_rpm); chama SetPeriod
MenuNav.cpp         → Start/Stop na entrada/saída da simulação
```

### Boas práticas ao alterar o Timer1

- Não colocar **implementações** em `Timer1_Direct.h` (causa *multiple definition* no link).
- Não definir outra `ISR(TIMER1_COMPA_vect)` (ex.: biblioteca TimerOne ou Servo).
- Ao mudar prescaler, atualize a fórmula em `Timer1_SetPeriod()` e a documentação acima.
- Testes de RPM alto: verifique se `TIMER1_MIN_US` ainda é seguro para o tipo de fônica ativo.

---

## Diagrama de Dependências

```
                    Config.h ←──────────────────────┐
                       ↑                             │
                    Tipos.h                          │
                    ↗     ↖                          │
            Sinais.h     MenuTextos.h                │
               ↑              ↑                      │
           Sinais.cpp    MenuData.h ← MenuData.cpp   │
               ↑              ↑                      │
          Sensores.h     MenuNav.h                   │
               ↑              ↑                      │
         Sensores.cpp   MenuNav.cpp ← Reles.cpp      │
               ↑              ↑         ↑            │
          Display.h      Display.cpp    Reles.h       │
               ↑              ↑                      │
    ISR_Timer1.cpp    Timer1_Direct.cpp              │
               ↑              ↑                      │
    Simulador_ECU_Final.ino ─────────────────────────┘
```

---

## Uso de Memória

| Recurso | Estratégia |
|---------|-----------|
| **Flash (PROGMEM)** | Strings de menu, tabelas de sinais e NTC em Flash via `PROGMEM` |
| **RAM** | Variáveis volatile compartilhadas com ISRs, buffers LCD (2 × 80 bytes) |
| **Timers** | Timer1 (CTC + ISR fônica/fase), Timer3/5 (CTC hardware VSS/MAP), Timer4 (ISR flag ADC) |
| **Pinos** | PORTA (26-29) via registrador direto, Timer outputs via CTC toggle |

> O ATmega2560 tem 256KB Flash e 8KB RAM. O projeto usa aproximadamente 30KB Flash e 2KB RAM.
