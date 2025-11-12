const mqtt = require('mqtt');
const Reading = require('../models/Reading');
const Device = require('../models/Device');
const Rule = require('../models/Rule');

const options = {
  host: 'wa2fc908.ala.us-east-1.emqxsl.com',
  port: 8883,
  protocol: 'mqtts',
  username: 'diovani',
  password: 'facco123'
};

const client = mqtt.connect(options);

// Mapeia modelo para tipo
const mapearTipo = (model) => {
  switch (model.toUpperCase()) {
    case 'KY-023': return 'joystick';
    case 'DHT11': return 'dht11';
    case 'MPU6050': return 'mpu6050';
    case 'DS18B20': return 'ds18b20';
    default: return model.toLowerCase();
  }
};

// Limpa tópicos retidos
const limparTopicosRetidos = async () => {
  try {
    const devices = await Device.find();
    const topicos = [];

    devices.forEach(device => {
      device.components?.forEach(c => {
        if (typeof c.model === 'string' && typeof c.pin === 'number') {
          const tipo = mapearTipo(c.model);
          const base = `sw${c.pin}`;
          topicos.push(`grupoX/sensor/${tipo}/${base}/position`);
          topicos.push(`grupoX/sensor/${tipo}/${base}/switch`);
        }
      });
    });

    topicos.forEach(t => {
      client.publish(t, '', { retain: true });
      console.log(`Mensagem retida limpa em: ${t}`);
    });
  } catch (err) {
    console.error('Erro ao limpar tópicos retidos:', err);
  }
};

client.on('connect', async () => {
  console.log('Conectado ao broker MQTT');
  await limparTopicosRetidos();

  client.subscribe('grupoX/config/response');
  client.subscribe('grupoX/sensor/#');
  client.subscribe('grupoX/atuador/botao');
});

// Buffer de últimas leituras
const ultimoValor = {};
const mudouSignificativamente = (a, b) => {
  if (a.x !== undefined && b.x !== undefined) {
    return Math.abs(a.x - b.x) > 10 || Math.abs(a.y - b.y) > 10;
  }
  return JSON.stringify(a) !== JSON.stringify(b);
};
const deveSalvar = (espId, novoValor) => {
  const anterior = ultimoValor[espId];
  const agora = Date.now();
  if (!anterior || agora - anterior.timestamp > 1000 || mudouSignificativamente(anterior.data, novoValor)) {
    ultimoValor[espId] = { data: novoValor, timestamp: agora };
    return true;
  }
  return false;
};

// --- Motor de Regras ---
const extractValue = (tipo, data, field = 'valor') => {
  // Se a regra especificou um campo, tenta extrair esse atributo
  if (field && data[field] !== undefined) {
    return parseFloat(data[field]);
  }

  // Se não tiver campo definido, tenta converter direto
  if (typeof data === 'number') return data;
  if (data.valor !== undefined) return parseFloat(data.valor);

  // Fallbacks por tipo de sensor
  switch (tipo) {
    case 'ds18b20':
      return parseFloat(data.temperature);
    case 'dht11':
      return parseFloat(data.temperatura_c ?? data.temperature);
    default:
      return NaN;
  }
};

const checkCondition = (op, v, a, b) => {
  switch (op) {
    case '>=': return v >= a;
    case '<=': return v <= a;
    case '==': return v == a;
    case '!=': return v != a;
    case '>':  return v > a;
    case '<':  return v < a;
    case 'between': return v >= a && v <= (b ?? a);
    default: return false;
  }
};

const publishAction = (action) => {
  const topic = `grupoX/atuador/${action.tipo}/${action.pino}`;
  client.publish(topic, action.command);
  console.log(`🚀 Regra acionada → ${topic}: ${action.command}`);

  const legacyTopic = `grupoX/sensor/${action.tipo}/sw${action.pino}/switch`;
  client.publish(legacyTopic, action.command);
  console.log(`🚀 Regra acionada (compatibilidade) → ${legacyTopic}: ${action.command}`);
};

// --- Processa mensagens MQTT ---
client.on('message', async (topic, message) => {
  const payload = message.toString();
  console.log("📩 Mensagem recebida:", { topic, payload });

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
    console.log("✅ Payload é JSON válido:", data);
  } catch {
    data = { valor: parseFloat(payload) };
    console.log("⚠️ Payload não era JSON, convertido para:", data);
  }

  console.log("📊 Dados interpretados:", data);

  const espId = `${tipo}_${pino}`;
  console.log("🔑 Identificador calculado:", espId);

  const podeSalvar = subtipo === 'switch' || deveSalvar(espId, data);
  console.log("💾 Deve salvar?", podeSalvar, "Subtipo:", subtipo);

  if (!podeSalvar) return;

  try {
    const reading = new Reading({ espId, tipo, pino, data, timestamp: new Date() });
    await reading.save();
    console.log(`[${tipo}] ✅ Leitura salva no pino ${pino}:`, data);
  } catch (err) {
    console.error(`❌ Erro ao salvar leitura de ${tipo} no pino ${pino}:`, err);
  }

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
    console.error("❌ Erro ao avaliar regras:", err);
  }
});

client.on('error', (err) => {
  console.error('❌ Erro MQTT:', err);
});

module.exports = client;
