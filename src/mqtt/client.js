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
  if (data[field] !== undefined) return parseFloat(data[field]);
  switch (tipo) {
    case 'ds18b20': return parseFloat(data.valor ?? data.temperature);
    case 'dht11': return parseFloat(data.temperatura_c ?? data.temperature);
    default: return parseFloat(data.valor);
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
  // Tópico principal da regra
  const topic = `grupoX/atuador/${action.tipo}/${action.pino}`;
  client.publish(topic, action.command);
  console.log(`Regra acionada → ${topic}: ${action.command}`);

  // 🔑 Compatibilidade: publica também no tópico que o ESP já escuta
  const legacyTopic = `grupoX/sensor/${action.tipo}/sw${action.pino}/switch`;
  client.publish(legacyTopic, action.command);
  console.log(`Regra acionada (compatibilidade) → ${legacyTopic}: ${action.command}`);
};

// --- Processa mensagens MQTT ---
client.on('message', async (topic, message) => {
  const payload = message.toString();

  if (topic === 'grupoX/config/response') {
    console.log('Confirmação de configuração recebida:', payload);
    return;
  }

  const parts = topic.split('/');
  if (parts.length < 4) return; // aceita tópicos com 4 ou mais partes

  const tipo = parts[2];
  const base = parts[3];
  const subtipo = parts[4] || 'default';

  const pino = Number(base.replace(/\D/g, ''));
  if (isNaN(pino)) return;

  let data;
  try {
    data = JSON.parse(payload);
  } catch {
    data = { valor: parseFloat(payload) };
  }

  const espId = `${tipo}_${pino}`;
  const podeSalvar = subtipo === 'switch' || deveSalvar(espId, data);
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
    const rules = await Rule.find({ deviceId: espId, "sensor.tipo": tipo, "sensor.pino": pino });
    for (const rule of rules) {
      const valor = extractValue(tipo, data, rule.sensor.field || 'valor');
      if (Number.isNaN(valor)) continue;
      const met = checkCondition(rule.condition.operator, valor, rule.condition.value, rule.condition.value2);
      if (met) publishAction(rule.action);
    }
  } catch (err) {
    console.error("Erro ao avaliar regras:", err);
  }
});

client.on('error', (err) => {
  console.error('Erro MQTT:', err);
});

module.exports = client;
