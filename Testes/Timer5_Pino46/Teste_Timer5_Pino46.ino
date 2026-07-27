/**
 * ============================================================================
 * Teste_Timer5_Pino46.ino — Teste isolado do Timer5 CTC no pino 46 (OC5A)
 * ============================================================================
 * 
 * Este sketch testa APENAS o Timer5 gerando onda quadrada no pino 46.
 * Não depende de nenhum outro módulo do simulador.
 * 
 * Upload para Arduino Mega 2560 e meça o pino 46 com osciloscópio.
 * 
 * Resultado esperado:
 *   - Onda quadrada ~120 Hz no pino 46 (default)
 *   - Serial Monitor mostra registradores e estado a cada 2s
 *   - Frequência muda automaticamente a cada 5s entre 95.9 Hz e 159 Hz
 * 
 * Hardware:
 *   - Arduino Mega 2560
 *   - Osciloscópio no pino 46
 *   - (opcional) LED+resistor no pino 46 para confirmação visual
 * 
 * NOTA SOBRE PINOS DO TIMER5:
 *   OC5A = PL3 = Arduino Mega pino 46  ← ESTE é o que usamos
 *   OC5B = PL4 = Arduino Mega pino 45  (NÃO usar)
 *   OC5C = PL5 = Arduino Mega pino 44  (NÃO usar)
 * ============================================================================
 */

#define VACUUM_PIN    46
#define F_CPU_HZ      16000000UL
#define PRESCALER_VAL 64

// ============================================================================
// TESTE 1: Configuração básica do Timer5 em modo CTC
// ============================================================================

void setupTimer5_CTC(float freq) {
  if (freq <= 0.0f) return;

  float ocr_f = ((float)F_CPU_HZ / (2.0f * (float)PRESCALER_VAL * freq)) - 1.0f;
  if (ocr_f < 1.0f || ocr_f > 65535.0f) return;
  uint16_t ocr = (uint16_t)ocr_f;

  // Configura pino como saída
  pinMode(VACUUM_PIN, OUTPUT);

  // Reset completo do timer
  TCCR5A = 0;
  TCCR5B = 0;
  TCNT5  = 0;

  // Configura registradores
  OCR5A  = ocr;                                         // TOP = OCR5A
  TCCR5A = (1 << COM5A0);                               // Toggle OC5A on compare
  TCCR5B = (1 << WGM52) | (1 << CS51) | (1 << CS50);   // CTC + prescaler 64

  Serial.print("Timer5 configurado: freq=");
  Serial.print(freq, 1);
  Serial.print(" Hz, OCR5A=");
  Serial.print(ocr);
  Serial.print(", freq_real=");
  float freq_real = (float)F_CPU_HZ / (2.0f * PRESCALER_VAL * (ocr + 1));
  Serial.print(freq_real, 2);
  Serial.println(" Hz");
}

// ============================================================================
// TESTE 2: Atualização segura do Timer5 (sem reset de TCNT)
// Simula o que o loop() rápido faz a cada ~400µs
// ============================================================================

void updateTimer5_Safe(float freq) {
  if (freq <= 0.0f) return;

  float ocr_f = ((float)F_CPU_HZ / (2.0f * (float)PRESCALER_VAL * freq)) - 1.0f;
  if (ocr_f < 1.0f || ocr_f > 65535.0f) return;
  uint16_t ocr = (uint16_t)ocr_f;

  const uint8_t CTC_PRESCALER_64 = (1 << WGM52) | (1 << CS51) | (1 << CS50);
  if ((TCCR5B & CTC_PRESCALER_64) == CTC_PRESCALER_64) {
    // Timer já rodando — só atualiza OCR se mudou
    if (OCR5A != ocr) {
      OCR5A = ocr;
      if (TCNT5 > ocr) TCNT5 = 0;
    }
    return;
  }

  // Primeira configuração
  setupTimer5_CTC(freq);
}

// ============================================================================
// TESTE 3: Versão BUGADA (reproduz o bug original)
// Reseta TCNT5 a cada chamada — demonstra por que não funciona
// ============================================================================

void updateTimer5_BUGGY(float freq) {
  if (freq <= 0.0f) return;

  float ocr_f = ((float)F_CPU_HZ / (2.0f * (float)PRESCALER_VAL * freq)) - 1.0f;
  if (ocr_f < 1.0f || ocr_f > 65535.0f) return;
  uint16_t ocr = (uint16_t)ocr_f;

  // BUG: reseta TUDO a cada chamada (como fazia o código refatorado)
  TCCR5A = 0;        // Desconecta OC5A
  TCCR5B = 0;        // Para o timer
  TCNT5  = 0;        // Reseta contador ← ESTE É O PROBLEMA
  OCR5A  = ocr;
  TCCR5A = (1 << COM5A0);
  TCCR5B = (1 << WGM52) | (1 << CS51) | (1 << CS50);
}

// ============================================================================
// Debug: imprime estado dos registradores
// ============================================================================

void printTimer5State() {
  Serial.println("--- Timer5 Registers ---");
  Serial.print("  TCCR5A = 0x"); Serial.println(TCCR5A, HEX);
  Serial.print("  TCCR5B = 0x"); Serial.println(TCCR5B, HEX);
  Serial.print("  TCNT5  = ");   Serial.println(TCNT5);
  Serial.print("  OCR5A  = ");   Serial.println(OCR5A);
  Serial.print("  OCR5B  = ");   Serial.println(OCR5B);
  Serial.print("  TIMSK5 = 0x"); Serial.println(TIMSK5, HEX);
  Serial.print("  Pin 46 state = ");  Serial.println(digitalRead(46));
  Serial.print("  DDRL bit3 = ");     Serial.println((DDRL >> 3) & 1);
  Serial.print("  COM5A = ");         Serial.println((TCCR5A >> 6) & 0x03);
  Serial.print("  WGM5  = ");
  uint8_t wgm = (TCCR5B >> 3) & 0x03;  // bits 3:2 of TCCR5B
  wgm = (wgm << 2) | (TCCR5A & 0x03);  // bits 1:0 of TCCR5A
  Serial.println(wgm);
  Serial.println("------------------------");
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println("==============================================");
  Serial.println("  TESTE ISOLADO: Timer5 CTC no Pino 46 (OC5A)");
  Serial.println("==============================================");
  Serial.println();

  // ---- FASE 1: Configuração inicial ----
  Serial.println("[FASE 1] Configurando Timer5 para 120 Hz...");
  setupTimer5_CTC(120.0f);
  printTimer5State();
  Serial.println("Verifique o pino 46 com osciloscópio. Deve ter ~120 Hz.");
  Serial.println("Aguardando 5 segundos...");
  Serial.println();
  delay(5000);

  // ---- FASE 2: Teste do update seguro (sem reset) ----
  Serial.println("[FASE 2] Mudando para 159 Hz via update SEGURO...");
  updateTimer5_Safe(159.0f);
  printTimer5State();
  Serial.println("Deve continuar gerando sinal, agora ~159 Hz.");
  Serial.println("Aguardando 5 segundos...");
  Serial.println();
  delay(5000);

  // ---- FASE 3: Teste do update seguro sem mudança de freq ----
  Serial.println("[FASE 3] Chamando update 1000× sem mudar frequência...");
  unsigned long t0 = micros();
  for (int i = 0; i < 1000; i++) {
    updateTimer5_Safe(159.0f);  // Deve retornar imediatamente (OCR não mudou)
  }
  unsigned long dt = micros() - t0;
  Serial.print("1000 chamadas em ");
  Serial.print(dt);
  Serial.println(" µs (deve ser rápido, ~100-200µs total)");
  printTimer5State();
  Serial.println("Sinal deve continuar estável em ~159 Hz.");
  Serial.println("Aguardando 5 segundos...");
  Serial.println();
  delay(5000);

  // ---- FASE 4: Demonstração do BUG ----
  Serial.println("[FASE 4] DEMONSTRANDO O BUG: chamando versão BUGADA 10000×...");
  Serial.println("Chamando updateTimer5_BUGGY(100 Hz) a cada 200µs...");
  Serial.println("Com OCR5A=1249, timer precisa de 5ms para toggle.");
  Serial.println("Mas resetamos TCNT a cada 200µs -> NUNCA atinge OCR5A!");
  
  // Para o timer antes de começar o teste bugado
  TCCR5A = 0; TCCR5B = 0;
  delay(100);
  
  t0 = millis();
  while (millis() - t0 < 5000) {
    updateTimer5_BUGGY(100.0f);  // Reseta TCNT5 a cada ~200µs
    delayMicroseconds(200);       // Simula loop rápido
  }
  
  Serial.println("RESULTADO: Pino 46 NÃO deve ter gerado sinal durante estes 5s!");
  Serial.println("(O osciloscópio deve mostrar linha reta ou ruído mínimo)");
  printTimer5State();
  Serial.println();

  // ---- FASE 5: Recuperação com update seguro ----
  Serial.println("[FASE 5] Recuperando com stopTimer5 + setupTimer5_CTC(95.9 Hz)...");
  TCCR5A = 0; TCCR5B = 0; // stop
  digitalWrite(VACUUM_PIN, LOW);
  delay(100);
  setupTimer5_CTC(95.9f);
  printTimer5State();
  Serial.println("Sinal deve voltar no pino 46 com ~95.9 Hz.");
  Serial.println();
  
  Serial.println("=== TESTE CONCLUÍDO ===");
  Serial.println("A partir de agora, o loop() chama updateTimer5_Safe()");
  Serial.println("a cada 500µs, variando entre 95.9 e 159 Hz a cada 5s.");
}

// ============================================================================
// LOOP — Simula o comportamento do loop() do simulador
// ============================================================================

void loop() {
  static unsigned long lastSwitch = 0;
  static unsigned long lastPrint = 0;
  static bool useHighFreq = false;

  float freq = useHighFreq ? 159.0f : 95.9f;

  // Simula chamada rápida (como o loop do simulador)
  updateTimer5_Safe(freq);

  // Troca frequência a cada 5s
  if (millis() - lastSwitch > 5000) {
    lastSwitch = millis();
    useHighFreq = !useHighFreq;
    Serial.print("[LOOP] Frequência → ");
    Serial.print(useHighFreq ? 159.0f : 95.9f, 1);
    Serial.println(" Hz");
  }

  // Imprime estado a cada 2s
  if (millis() - lastPrint > 2000) {
    lastPrint = millis();
    Serial.print("[LOOP] TCNT5=");
    Serial.print(TCNT5);
    Serial.print(" OCR5A=");
    Serial.print(OCR5A);
    Serial.print(" Pin46=");
    Serial.println(digitalRead(46));
  }

  // Pequeno delay para simular tempo de analogRead no loop real
  delayMicroseconds(400);
}
