# 📋 Changelog — Simulador de ECU

Todas as alterações notáveis neste projeto estão documentadas aqui.

---

## [2.8.6] — 2026-07-30

### Corrigido
- **Fônica “cortando”** — ao fim da volta o contador `_cP` era zerado num tick sem gerar flanco, alongando a falha em 1 passo; agora a volta reinicia no mesmo tick.
- **Hall** — removido o passo morto extra entre meio-ciclos.
- **Timer1_SetPeriod** — atualização atômica de OCR1A/TCNT para reduzir glitches ao mudar RPM.

---

## [2.8.5] — 2026-07-27

### Adicionado
- **Menu Diesel Pesado** — MB, Scania, Iveco, Volvo, Cummins, MWM, MAN/VW.
- **14 sinais** (índices 70–83): PLD/MR, EMS S6, MS6.2, MS6.2/6.3 Iveco, EDC7 FPT, D12/D13, EDC7 ISB 4/6, EDC7C1 4/6, UC31, EDC7 C32 — todos **60-2** sem fase (nível A).

---

## [2.8.4] — 2026-07-27

### Adicionado
- **Menu Diesel Leve** — marcas VW, GM, Ford, Fiat, PSA, MB, Nissan, Toyota, Troller, Iveco.
- **14 sinais** (índices 56–69): EDC16C8, EDC17 Amarok, EDC16C9 S10, SID901/208 Ranger, EDC15/16/17 Fiat, EDC15C7 PSA, EDC15C6 Sprinter, EDC16 Frontier, Denso Hilux, SID901 Troller, EDC16 Daily — todos **60-2** sem fase (nível A).

---

## [2.8.3] — 2026-07-27

### Corrigido
- **VSS no LCD** — o monitor exibia `_vss / 2`, metade da velocidade usada no Timer3; painéis automotivos liam o valor correto. Agora LCD e frequência usam o mesmo km/h (0–200).

---

## [2.8.2] — 2026-07-27

### Adicionado
- **Preenchimento parcial `sinais[]` (nível A)** — Ford 35–38 (36-1), GM 39–43 (60-2), Peugeot 44–48 (60-2), Renault 49–55 (60-2); `tipFse=0` (sem fase calibrada).
- Menus Ford/GM/Peugeot/Renault passam a iniciar o monitor (`AtivaMonitorAnalogico`).

### Corrigido
- **Citroën ME7.4.4** — `tipSin` 44-4 → **60-2** (padrão Bosch documentado).

### Pendente
- **Honda / Toyota** — Honda tipicamente 12+1 (fora de `baseSinais[]`); Toyota sem entrada.

---

## [2.8.1] — 2026-07-27

### Alterado
- **Menu Carros reorganizado** no padrão comercial (ECU TEC / ECUPROG): itens como **módulo + referência** (ex.: `IAW 4AFB Fire`, `Bosch ME7.5.x`).
- **Ordem das marcas** por volume BR: VW → Fiat → GM → Ford → Renault → Peugeot → Citroën → Honda → Toyota → BMW → Chrysler.

---

## [2.8.0] — 2026-07-25

### Adicionado
- **EEPROM interna** — restaura último sinal, modo RPM, fase customizada e inversões; estrutura validada por magic, versão e checksum.
- **Editor de fase no LCD** — ajusta quantidade e início/fim de até quatro dentes, com salvar/cancelar e restauração da tabela original.
- **Modos de RPM** — potenciômetro, valores fixos de 800/2000/4000 RPM e varredura automática de 800 a 5000 RPM.
- **Parser Serial** — comandos `status`, `dump`, `sinal=`, `rpm=`, `faseN=`, `resetfase`, `pot`, `fixo`, `sweep`, `invertfonica` e `invertfase`.
- **Inversão lógica** independente das saídas de fônica e fase.

### Segurança e compatibilidade
- Atualização atômica dos parâmetros lidos pela ISR e reset do contador de fase.
- `EEPROM.put()` evita regravação de bytes sem mudança.
- Editor bloqueado para sinais Hall sem ciclo de fase.

---

## [2.7.2] — 2026-07-25

### Corrigido
- **Índices do menu ↔ `sinais[]`** — Audi (ATF…Simos), BMW, Chrysler, Citroën e Fiat apontavam para a linha errada (deslocamento de 1).
- **Seleção de veículo** — BMW/Chrysler/Citroën/Fiat passam a usar `AtivaMonitorAnalogico` (antes só `SelecionaSin`, sem iniciar o monitor).
- **Fiat 500 1.4** — `tipFse=3` com dentes zerados; preenchido com padrão aproximado do Fire (mesma família de motor).

## [2.7.1] — 2026-05-23

### Corrigido
- **Sinal de RPM (Timer1) com frequência errada** — `calcPeriodo()` usava multiplicador `10000` em vez de `1000000`, deixando a fônica ~100× mais rápida (período mínimo de 50 µs quase o tempo todo).
- **Limite `TIMER1_MAX_US`** — aumentado de 10 ms para 262 ms (máximo do OCR1A), permitindo RPM baixo real.
- **Passos por volta** — fônica usa `_falha` da tabela de sinais (74 para 36-1, 96 para 44-4, 120 para 60-2) em vez de constantes fixas.
- **Início da simulação** — `AtivaMonitorAnalogico()` lê o potenciômetro e aplica o período correto antes de `Timer1_Start()`.

### Documentação (2026-05-25)
- **README.md**, **GUIA_RAPIDO.md**, **ARQUITETURA.md** e **COMO_ADICIONAR_SENSORES.md** alinhados à v2.7.1.
- Seção Timer1 atualizada: `calcTempoTimer1()`, fórmula `calcPeriodo`, `TIMER1_MAX_US` 262 ms, leitura de A0 em `AtivaMonitorAnalogico()`.
- Guia rápido: faixa RPM 0–5100, troubleshooting de frequência incorreta.
- Guia de expansão: cálculo de período centralizado em `Sensores.cpp` (não mais no `.ino`).

---

## [2.7] — 2026-05-23

### Alterado
- **Timer1 sem biblioteca TimerOne** — Controle direto em `Timer1_Direct.cpp`; ISR em `ISR_Timer1.cpp` para evitar conflito de símbolos no linker.
- **Documentação do Timer1** — Seção dedicada em `ARQUITETURA.md` (hardware CTC, fórmula OCR1A, RPM→período, ciclo de vida, diagrama); resumo em `README.md`; comentários em `Config.h` e nos fontes do Timer1.

### Corrigido
- **Compilação** — Implementações removidas de `Timer1_Direct.h` (erro *multiple definition* ao incluir o header em `.ino` e `MenuNav.cpp`).

---

## [2.6] — 2025-05-23

### Corrigido (Causa Raiz)
- **Timer3 (VSS) e Timer5 (MAP) nunca geravam sinal** — A refatoração substituiu `lePorta()` (~60ms × 3 leituras = 180ms por iteração do loop) por `analogRead()` (~112µs × 3 = 336µs), acelerando o loop **450×**. As funções `setupTimer3_VSS()` e `setVacuumFreq()` faziam reinicialização completa dos registradores (`TCCR=0, TCNT=0`) a cada chamada. O reset de `TCNT` a cada ~400µs impedia o contador de atingir `OCR` (3-5ms para MAP, 1-23ms para VSS), fazendo os toggles `OC3A`/`OC5A` nunca acontecerem.

### Solução
- **Padrão "detect-if-running"** — Ambas as funções agora verificam se o timer já está configurado em modo CTC com prescaler 64. Se sim, apenas atualizam `OCR` quando o valor muda, sem resetar `TCNT` nem reconfigurar registradores. O timer roda continuamente, produzindo sinal estável.

### Adicionado
- Sketch de teste isolado `Testes/Timer5_Pino46/Teste_Timer5_Pino46.ino` com 5 fases de diagnóstico
- Documentação completa do debug em `DEBUG_MAP_PWM.md` (não incluído na versão final — consultar repositório de debug)

---

## [2.5] — 2025-05-22

### Corrigido
- **Flags `vssAtivo`/`mapHzAtivo` nunca ativados** (CRÍTICO) — A v2.4 removeu `vssAtivo = true; mapHzAtivo = true;` de `AtivaMonitorAnalogico()`. Sem estas flags, o `loop()` sempre executava `stopTimer3()`/`stopTimer5()`, impedindo completamente a geração de sinais VSS e MAP.
  - **Arquivo:** `MenuNav.cpp`
  - **Correção:** Restaurados ambos os flags em `AtivaMonitorAnalogico()`

- **Timer5 MAP usava canal errado** (CRÍTICO) — Código configurava `OCR5B` e `COM5B0` (canal B, pino 45), mas o pino 46 do Arduino Mega é `OC5A` (canal A). Em modo CTC (`WGM52`), TOP = `OCR5A`. Como `OCR5A` ficava em 0 (valor default), o timer resetava a cada clock — nenhum sinal era gerado.
  - **Arquivo:** `Sensores.cpp`
  - **Correção:** `OCR5B` → `OCR5A`, `COM5B0` → `COM5A0`

- **`AtivaMonitorAnalogico()` ignorava parâmetro `param`** (MODERADO) — Sempre usava `valParametro` (global), ignorando o argumento. Para itens de menu com seleção direta (ex: Audi ATF com `param=6`), `valParametro` era 0 → `SelecionaSin(0)` retornava sem configurar nada.
  - **Arquivo:** `MenuNav.cpp`
  - **Correção:** `uint8_t sinalIdx = (param > 1) ? param : valParametro;`

### Removido
- Blocos de comentário duplicados em `Config.h` e `Sensores.cpp`

---

## [2.3] — Data anterior

### Alterado
- Entradas analógicas A2-A9, A12, A13 configuradas para divisor resistivo 5:1 (0-25V)
- Conversão ADC corrigida: multiplicar por 5 para recuperar voltagem real
- Sensores NTC (ACT/ECT) recebem voltagem de 0-25V
- TPS recebe 0-25V e mapeia 0-100%
- MAP recebe 0-25V e mapeia 0-101.325 kPa
- Potenciômetros (A0, A1, A10) mantidos em 0-5V direto

---

## [2.2] — Data anterior

### Corrigido
- **ISR Timer4 não-bloqueante** — `analogRead()` movido para `loop()` (antes: 11 leituras dentro da ISR = ~1.1ms com interrupts desabilitados, bloqueando Timer1)
- Variáveis compartilhadas com ISR declaradas `volatile`
- `F_CPU_HZ` corrigido para 16 MHz (era 160.000 — fator 100× errado)
- Proteções contra NaN/Inf em `setupTimer3`/`Timer5`
- Tabela NTC expandida para -40°C a 150°C (19 pontos)
- Proteção contra overflow de pilha de menu
- Validação NULL em `strncpy_P()`
- Comparação segura em `lcdWriteIfChanged()`

---

## [2.0] — Data anterior

### Adicionado
- Refatoração do código monolítico para estrutura modular (9 módulos)
- Menu hierárquico com scroll e pilha de navegação
- Monitor analógico com smart-update (só reescreve mudanças no LCD)
- Geração de sinais VSS e MAP por hardware timer (CTC)
- Suporte a 34+ veículos pré-configurados
