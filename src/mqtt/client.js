// --- Processa mensagens MQTT ---
client.on('message', async (topic, message) => {
  const payload = message.toString();
  console.log("📩 Mensagem recebida:", { topic, payload });   // <-- log inicial

  if (topic === 'grupoX/config/response') {
    console.log('Confirmação de configuração recebida:', payload);
    return;
  }

  const parts = topic.split('/');
  if (parts.length < 4) {
    console.log("⚠️ Tópico ignorado, partes insuficientes:", parts);
    return;
  }

  const tipo = parts[2];
  const base = parts[3];
  const subtipo = parts[4] || 'default';

  const pino = Number(base.replace(/\D/g, ''));
  if (isNaN(pino)) {
    console.log("⚠️ Pino inválido extraído de base:", base);
    return;
  }

  let data;
  try {
    data = JSON.parse(payload);
  } catch {
    data = { valor: parseFloat(payload) };
  }
  console.log("📊 Dados interpretados:", data);

  const espId = `${tipo}_${pino}`;
  console.log("🔑 Identificador calculado:", espId);

  const podeSalvar = subtipo === 'switch' || deveSalvar(espId, data);
  console.log("💾 Deve salvar?", podeSalvar, "Subtipo:", subtipo);

  if (!podeSalvar) return;

  // --- SEMPRE salva leitura no banco ---
  try {
    const reading = new Reading({ espId, tipo, pino, data, timestamp: new Date() });
    await reading.save();
    console.log(`[${tipo}] Leitura salva no pino ${pino}:`, data);
  } catch (err) {
    console.error(`Erro ao salvar leitura de ${tipo} no pino ${pino}:`, err);
  }

  // --- Motor de Regras ---
  try {
    console.log("🔍 Buscando regras com filtro:", { deviceId: espId, "sensor.tipo": tipo, "sensor.pino": pino });
    const rules = await Rule.find({ deviceId: espId, "sensor.tipo": tipo, "sensor.pino": pino });
    console.log("📋 Regras encontradas:", rules.length);

    for (const rule of rules) {
      console.log("➡️ Avaliando regra:", rule.name);
      const valor = extractValue(tipo, data, rule.sensor.field || 'valor');
      console.log("📐 Valor extraído:", valor);

      if (Number.isNaN(valor)) {
        console.log("⚠️ Valor inválido (NaN), regra ignorada");
        continue;
      }

      const met = checkCondition(rule.condition.operator, valor, rule.condition.value, rule.condition.value2);
      console.log(`📏 Condição ${rule.condition.operator} ${rule.condition.value} →`, met);

      if (met) {
        console.log("✅ Condição satisfeita, publicando ação:", rule.action);
        publishAction(rule.action);
      } else {
        console.log("❌ Condição não satisfeita");
      }
    }
  } catch (err) {
    console.error("Erro ao avaliar regras:", err);
  }
});
