# Teste Isolado — Timer5 CTC no Pino 46 (OC5A)

Sketch de diagnóstico independente para verificar o funcionamento do Timer5
na geração de sinal MAP PWM.

## ⚠️ Compilação Separada

Este sketch **NÃO** deve ser compilado junto com o projeto principal.
Possui suas próprias funções `setup()` e `loop()`.

## Como usar

1. Feche o projeto principal na Arduino IDE
2. Abra este arquivo (`Teste_Timer5_Pino46.ino`)
3. Selecione Arduino Mega 2560
4. Compile e faça Upload
5. Abra Serial Monitor (9600 baud)
6. Conecte osciloscópio ao pino 46

## Fases do teste

| Fase | Descrição | Resultado esperado |
|------|-----------|--------------------|
| 1 | Configura 120 Hz | Onda quadrada no pino 46 |
| 2 | Muda para 159 Hz | Frequência muda suavemente |
| 3 | 1000 chamadas repetidas | Sinal estável, execução rápida |
| 4 | Demonstra o bug (reset TCNT a cada 200µs) | Sem sinal no pino 46 |
| 5 | Recuperação com 95.9 Hz | Sinal volta normalmente |
