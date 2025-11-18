// src/mqtt/client.js

const mqtt = require('mqtt');
const Device = require('../models/Device');
const Reading = require('../models/Reading');
const Rule = require('../models/Rule');

// -------------------------------------------------
// CONFIG MQTT
// -------------------------------------------------

const options = {
  host: 'wa2fc908.ala.us-east-1.emqxsl.com',
  port: 8883,
  protocol: 'mqtts',
  username: 'diovani',
  password: 'facco123'
};

const client = mqtt.connect(options);

client.on('connect', () => {
  console.log("🚀 Conectado ao broker MQTT");

  client.subscribe('grupoX/sensor/#');
  client.subscribe('grupoX/config/response');
});

// -------------------------------------------------
// MAPEAMENTO DO TIPO DO MQTT → MODEL DO BANCO
// -------------------------------------------------

// O ESP publica "joystick" → no banco está "KY-023"
// O ESP publica "led" → no banco está "LED"
const mapTipoToModel = (tipo) => {
  const map = {
    joystick: "KY-023",
    joystick_ky023: "KY-023",
    led: "LED",
    botao: "BOTAO",
    encoder: "ENCODER",
    rele: "RELE",
    motor_vibracao: "MOTOR_VIBRACAO",
    dht11: "DHT11",
    dht22: "DHT22",
    ds18b20: "DS18B20",
    hcsr04: "HCSR04",
    mpu6050: "MPU6050",
    apds9960: "APDS9960",
    keypad4x4: "KEYPAD4X4"
  };

  return map[tipo.toLowerCase()] || tipo.toUpperCase();
};

// -------------------------------------------------
// EXTRACT VALUE
// -------------------------------------------------

const extractValue = (data, field) => {
  if (!data || !field) return null;
  return data[field] !== undefined ? data[field] : null;
};

// -------------------------------------------------
// CHECK DE CONDIÇÃO
// -------------------------------------------------

const checkCondition = (op, v, a, b) => {
  if (v === null || v === undefined) return false;

  switch (op) {
    case '>=': return v >= a;
    case '<=': return v <= a;
    case '>': return v > a;
    case '<': return v < a;
    case '==': return v == a;
    case '!=': return v != a;
    case 'between': return v >= a && v <= b;
    default: return false;
  }
};

// -------------------------------------------------
// PUBLICAÇÃO DO ATUADOR
// -------------------------------------------------

const publishAction = (action) => {
  const topic = `grupoX/atuador/${action.tipo}/${action.pino}`;
  client.publish(topic, action.command);

  console.log(`⚡ Atuador acionado → ${topic}: ${action.command}`);
};

// -------------------------------------------------
// PROCESSAMENTO DA MENSAGEM MQTT
// -------------------------------------------------

client.on('message', async (topic, msg) => {
  try {
    const payload = msg.toString();
    console.log("\n📩 Mensagem recebida:", topic, payload);

    if (topic === 'grupoX/config/response') {
      console.log("🔧 Resposta de config:", payload);
      return;
    }

    const parts = topic.split('/');

    if (parts.length < 3) return;

    // grupoX/sensor/<tipo>/xxxx
    const tipoBruto = parts[2];
    const modelEsperado = mapTipoToModel(tipoBruto);

    // -------------------------------------------------
    // EXTRAIR PINO
    // -------------------------------------------------

    let pino = null;

    // Caso normal:
    // grupoX/sensor/dht11/12
    if (parts.length === 4) {
      pino = Number(parts[3]);
    }

    // Joystick:
    // grupoX/sensor/joystick/sw25/position
    else if (parts.length >= 5) {
      const identifier = parts[3];
      pino = Number(identifier.replace(/\D/g, ''));
    }

    if (isNaN(pino)) {
      console.log("❌ Não foi possível extrair pino do tópico:", topic);
      return;
    }

    // -------------------------------------------------
    // PARSE DO PAYLOAD
    // -------------------------------------------------

    let data;
    try {
      data = JSON.parse(payload);
    } catch {
      data = { valor: payload };
    }

    // -------------------------------------------------
    // ENCONTRAR DEVICE CORRESPONDENTE (CORREÇÃO FINAL)
    // -------------------------------------------------

    const device = await Device.findOne({
      components: {
        $elemMatch: {
          model: modelEsperado,
          pin: pino
        }
      }
    });

    if (!device) {
      console.log(`⚠ Nenhum device com model=${modelEsperado}, pin=${pino}`);
      return;
    }

    const espId = device.espId;

    // -------------------------------------------------
    // SALVAR LEITURA
    // -------------------------------------------------

    await new Reading({
      espId,
      tipo: tipoBruto,
      pino,
      data
    }).save();

    console.log(`💾 Salvo para ESP ${espId} → ${tipoBruto} (${pino})`, data);

    // -------------------------------------------------
    // BUSCAR REGRAS
    // -------------------------------------------------

    const rules = await Rule.find({
      deviceId: espId,
      "sensor.tipo": tipoBruto,
      "sensor.pino": pino
    });

    if (rules.length === 0) {
      console.log("📭 Nenhuma regra para este sensor.");
      return;
    }

    console.log(`📋 ${rules.length} regras encontradas para ${tipoBruto}, pino ${pino}`);

    // -------------------------------------------------
    // EXECUTAR REGRAS
    // -------------------------------------------------

    for (const rule of rules) {
      console.log(`➡ Avaliando regra: ${rule.name}`);

      const valor = extractValue(data, rule.sensor.field);

      console.log("🔎 Valor extraído:", valor);

      const ok = checkCondition(
        rule.condition.operator,
        valor,
        rule.condition.value,
        rule.condition.value2
      );

      if (ok) {
        console.log("✅ Condição satisfeita → executando ação");
        publishAction(rule.action);
      } else {
        console.log("❌ Condição não satisfeita");
      }
    }

  } catch (err) {
    console.error("❌ Erro no processamento MQTT:", err);
  }
});

client.on('error', (e) => console.error("Erro MQTT:", e));

module.exports = client;