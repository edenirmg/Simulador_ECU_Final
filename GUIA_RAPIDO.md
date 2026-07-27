# ⚡ Guia Rápido — Simulador de ECU v2.8.0

## 1. Primeiro Uso

### Montagem
1. Conecte o display LCD 20×4 via I2C (SDA/SCL)
2. Conecte os 4 botões nos pinos 22-25 (com pull-up interno)
3. Conecte 3 potenciômetros: A0 (RPM), A1 (VSS), A10 (MAP)
4. Conecte os 3 relés nos pinos 50-52

### Upload
1. Abra `Simulador_ECU_Final.ino` na Arduino IDE
2. Selecione **Arduino Mega 2560** como placa
3. Clique em **Upload**

### Primeira Execução
O LCD exibe o menu principal:
```
 MENU PRINCIPAL
→Carros
 Sinais Basicos
 Hall Magnetico
```

---

## 2. Navegação no Menu

| Botão | Pino | Função |
|-------|------|--------|
| **Subir** | 24 | Move cursor para cima |
| **Descer** | 22 | Move cursor para baixo |
| **Enter** | 23 | Seleciona item / entra no submenu |
| **Voltar** | 25 | Volta ao menu anterior / para simulação |

### Estrutura do Menu

```
Menu Principal
├── Carros
│   ├── VW/Audi/Seat → IAW 1AVP, 4BV, ME7.5, MP9, Simos…
│   ├── Fiat → IAW 4AFB Fire, ME7.9.9 Etorq, 59FB…
│   ├── GM/Chevrolet → Multec H, M1.5.x, ME7.9.9…
│   ├── Ford → EEC-V/VI, IAW 4CFR (36-1, sem fase)
│   ├── Renault → Sirius, Fenix, IAW 5NR (60-2, sem fase)
│   ├── Peugeot / Citroën → ME7.4.x, IAW 5NP/6LP/8P
│   ├── Honda (pendente 12+1)
│   ├── Toyota (pendente)
│   ├── BMW / Chrysler
├── Diesel Leve
│   ├── VW → EDC16C8 Delivery, EDC17 Amarok
│   ├── GM → EDC16C9 S10
│   ├── Ford → SID901 / SID208 Ranger
│   ├── Fiat → EDC15 / EDC16C39 Ducato, EDC17C69 Toro
│   ├── PSA / MB / Nissan / Toyota / Troller / Iveco
├── Diesel Pesado
│   ├── Mercedes → PLD/MR Temic
│   ├── Scania → EMS S6/PDE, MS6.2
│   ├── Iveco → MS6.2 / MS6.3 / EDC7 FPT
│   ├── Volvo → D12 / D13
│   ├── Cummins → EDC7 ISB 4/6 cil
│   ├── MWM → EDC7C1 4/6, UC31 EuroV
│   └── MAN/VW → EDC7 C32
├── Sinais Básicos
│   ├── Hall Contínuo
│   └── 36-1 / 36-2 / 44-4 / 60-2
└── Ajustes
    ├── RPM Potenciômetro / Fixo / Varredura
    ├── Editar / Restaurar Fase
    └── Inverter Fônica / Fase
```

Diesel leve/pesado usa **60-2** sem fase calibrada (nível A). Ajuste de fase depois via **Ajustes → Editar Fase**.

Itens de carro usam o padrão comercial **módulo + referência** (como ECUPROG / ECU TEC), não só o nome do motor.

### Editor de fase
Selecione primeiro um veículo e volte ao menu. Em **Ajustes → Editar Fase**:
- **Subir/Descer** altera o valor
- **Enter** avança e salva após o último campo
- **Voltar** cancela e restaura os valores anteriores

Sinais Hall sem ciclo de fase não podem ser editados.

---

## 3. Simulando um Veículo

### Passo a passo
1. Navegue até **Carros → VW/Audi/Seat → Bosch ME7.5.x** (ou outro módulo)
2. Pressione **Enter**
3. O display muda para o **Monitor Analógico**:

```
 RPM:1234  Fase:1
→VSS:  50.0 Km/h
 MAP: 80.5 kPa
 MAP PWM: 45.2 kPa
```

### Controles durante simulação

| Potenciômetro | Pino | Controle |
|---------------|------|----------|
| **RPM** | A0 | Velocidade de rotação (0-5100 RPM) |
| **VSS** | A1 | Velocidade do veículo (0-200 km/h) |
| **MAP** | A10 | Pressão de vácuo (0-600 mmHg) |

### Saídas de sinal
- **Pino 26** → Sinal de fônica/Hall (onda quadrada)
- **Pino 27** → Sinal de fase
- **Pino 5** → VSS (frequência proporcional à velocidade)
- **Pino 46** → MAP PWM (frequência proporcional ao vácuo)

### Parar a simulação
Pressione **Voltar** — todos os timers param, relés desligam.

---

## 4. Monitor Analógico

Use **Subir/Descer** durante a simulação para navegar entre as variáveis:

| # | Variável | Unidade | Fonte |
|---|----------|---------|-------|
| 0 | VSS | km/h | Pot A1 |
| 1 | MAP | kPa | Sensor A5 |
| 2 | MAP PWM | kPa | Pot A10 |
| 3 | ACT | °C | Sensor A2 |
| 4 | ECT | °C | Sensor A3 |
| 5 | TPS | % | Sensor A4 |
| 6-9 | 5V Sensores (1-4) | V | A6-A9 |
| 10-11 | Sonda Lambda (1-2) | V | A11, A12 |

---

## 5. Tipos de Sinal

### Hall Contínuo
Onda quadrada simples. Frequência controlada pelo pot RPM.
- Uso: sensores de posição do eixo de comando

### Fônica 36-1
Roda com 36 dentes e 1 falha. Sinal com 36 pares de HIGH/LOW seguidos de 1 falha (HIGH prolongado).
- Uso: Audi, BMW

### Fônica 60-2
Roda com 60 dentes e 2 falhas. Mais comum em veículos GM, VW, Fiat.
- Uso: VW Gol, GM Corsa, Fiat Palio

### Fônica 44-4
Roda com 44 dentes e 4 falhas.
- Uso: Renault, Nissan

### Fase
Sinal de referência do eixo de comando (1 pulso por volta). Configurável de 1 a 4 dentes com posição e largura individuais.

---

## 6. Configuração de Relés

O menu **Hall Magnético** permite ajustar:
- **Hall Magnético / Indutivo** — Seleciona tipo de saída (relé pino 50)
- **5V / 12V** — Seleciona tensão do sinal Hall (relé pino 51)

---

## 7. Comandos Serial e EEPROM
No monitor a **9600 baud**, envie linhas como:
- `status`, `dump`
- `sinal=12`, `rpm=2500`, `pot`, `fixo`, `sweep`
- `fase1=32,72`, `resetfase`
- `invertfonica`, `invertfase`

O último sinal, modo RPM, fase customizada e polaridades são restaurados pela EEPROM ao reiniciar.

---

## 8. Teste Isolado do Timer5

<a name="teste-isolado-timer5"></a>

> ⚠️ **Compilar separadamente!** O sketch de teste tem suas próprias funções `setup()`/`loop()`.

### Localização
```
Testes/Timer5_Pino46/Teste_Timer5_Pino46.ino
```

### Como usar
1. **Feche** o projeto principal na Arduino IDE
2. Abra `Testes/Timer5_Pino46/Teste_Timer5_Pino46.ino`
3. Selecione Arduino Mega 2560
4. Compile e faça Upload
5. Abra Serial Monitor (9600 baud)
6. Conecte osciloscópio ao pino 46

### O que o teste faz
O sketch executa 5 fases automaticamente:

| Fase | Descrição | Resultado esperado |
|------|-----------|--------------------|
| 1 | Configura 120 Hz | Onda quadrada no pino 46 |
| 2 | Muda para 159 Hz | Frequência muda |
| 3 | 1000 chamadas iguais | Sinal estável |
| 4 | **Demonstra o bug** (reset a cada 200µs) | Sem sinal |
| 5 | Recuperação com 95.9 Hz | Sinal volta |

Após as fases, o `loop()` alterna entre 95.9 e 159 Hz a cada 5 segundos.

---

## 8. Solução de Problemas

### Display em branco
- Verifique o endereço I2C: `Config.h` → `LCD_ENDERECO` (default: 0x20)
- Execute um I2C scanner para encontrar o endereço correto

### Sem sinal de fônica (pino 26)
- Verifique se um veículo foi selecionado no menu
- Use osciloscópio para confirmar atividade no pino

### VSS sem sinal (pino 5)
- Gire o potenciômetro A1 acima de ~1%
- Verifique com osciloscópio

### MAP sem sinal (pino 46)
- Gire o potenciômetro A10
- Verifique que `mapHzAtivo == true` (Serial Monitor)
- Execute o sketch de teste isolado para validar hardware

### RPM não responde ao potenciômetro
- Verifique conexão de A0
- Verifique que o sinal correto foi selecionado (Hall vs Fônica)

### RPM/fônica com frequência errada (muito rápida ou travada no mínimo)
- Confirme que está na **v2.7.1** ou superior (correção de `calcPeriodo` e `_falha`)
- Abra o Serial Monitor (9600 baud): ao iniciar a simulação deve aparecer `Timer1 iniciado - Periodo: … us RPM: …`
- Com RPM baixo (~500), o período deve ser da ordem de **dezenas de ms**, não 50 µs
- Se o período ficar sempre em 50 µs, o potenciômetro A0 pode estar desconectado ou o veículo não foi selecionado
