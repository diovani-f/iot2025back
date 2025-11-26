// src/mqtt/client.js
const mqtt = require('mqtt');
const Device = require('../models/Device');
const Reading = require('../models/Reading');
const Rule = require('../models/Rule');

const options = {
  host: 'wa2fc908.ala.us-east-1.emqxsl.com',
  port: 8883,
  protocol: 'mqtts',
  username: 'diovani',
  password: 'facco123'
};

const client = mqtt.connect(options);

client.on('connect', () => {
  console.log("Conectado ao broker MQTT");
  client.subscribe('grupoX/sensor/#');
  client.subscribe('grupoX/config/response');
  client.subscribe('grupoX/+/status'); 
});

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
    keypad4x4: "KEYPAD4X4",
    ir_receiver: "IR_RECEIVER"
  };
  if (!tipo) return null;
  const key = tipo.toLowerCase();
  return map[key] || tipo.toUpperCase();
};

const extractValue = (data, field) => {
  if (!data || !field) return null;

  if (field.includes('.')) {
    const parts = field.split('.');
    let cur = data;
    for (const p of parts) {
      if (cur === undefined || cur === null) return null;
      cur = cur[p];
    }
    return cur !== undefined ? cur : null;
  }

  const aliases = {
    // temperature
    temperature: ["temperature", "temperatura_c", "tempC", "temp_c", "temp", "temperatura", "valor"],
    // humidity
    humidity: ["humidity", "umidade_pct", "umidade", "humidity_pct"],
    // distance (ultrasonic)
    distance: ["distancia_cm", "distance_cm", "dist_cm", "distance"],
    // infrared
    code: ["codigo_hex", "code", "codigo"],
    // encoder
    pps: ["pps", "pps_value"],
    // joystick
    x: ["x", "pos_x", "posy"],
    y: ["y", "pos_y", "posy"],
    click: ["click", "evento", "pressed", "pressionado"],
    // keypad
    tecla: ["tecla", "key", "keyPressed"],
    // accel/gyro
    acelerometro: ["acelerometro", "accelerometer", "accel"],
    giroscopio: ["giroscopio", "gyroscope", "gyro"],
    // generic value
    value: ["value", "valor"]
  };

  const lowerField = field.toLowerCase();
  if (aliases[lowerField]) {
    for (const k of aliases[lowerField]) {
      if (data[k] !== undefined) return data[k];
      // try case variations
      if (data[k.toLowerCase()] !== undefined) return data[k.toLowerCase()];
      if (data[k.toUpperCase()] !== undefined) return data[k.toUpperCase()];
    }
  }

  // try direct hit
  if (data[field] !== undefined) return data[field];
  if (data[lowerField] !== undefined) return data[lowerField];
  if (data[field.toLowerCase()] !== undefined) return data[field.toLowerCase()];

  // Try to find a numeric value if the payload is simple like { "value": 12 }
  if (data.value !== undefined) return data.value;
  if (data.valor !== undefined) return data.valor;

  console.log(`[EXTRACT] Failed to extract '${field}' from:`, JSON.stringify(data));
  return null;
};

const checkCondition = (op, v, a, b) => {
  if (v === null || v === undefined) return false;

  // ensure numeric comparisons coerce if possible
  const numV = (typeof v === 'string' && !isNaN(Number(v))) ? Number(v) : v;
  const numA = (typeof a === 'string' && !isNaN(Number(a))) ? Number(a) : a;
  const numB = (typeof b === 'string' && !isNaN(Number(b))) ? Number(b) : b;

  switch (op) {
    case '>=': return numV >= numA;
    case '<=': return numV <= numA;
    case '>': return numV > numA;
    case '<': return numV < numA;
    case '==': return numV == numA;
    case '!=': return numV != numA;
    case 'between': return numV >= numA && numV <= numB;
    case 'in': return Array.isArray(a) && a.includes(v);
    case 'notin': return Array.isArray(a) && !a.includes(v);
    case 'contains': return (typeof v === 'string' || Array.isArray(v)) && v.includes(a);
    default: return false;
  }
};

const publishAction = (action, targetDeviceId) => {
  if (!action) return;
  if (Array.isArray(action)) {
    for (const act of action) publishAction(act, targetDeviceId);
    return;
  }

  const tipo = action.tipo;
  const cmd = action.command;
  const pinos = Array.isArray(action.pino) ? action.pino : [action.pino];

  if (tipo === undefined || pinos.length === 0 || cmd === undefined || !targetDeviceId) {
    console.log("Ação inválida, faltando tipo/pino/command ou targetDeviceId:", action, targetDeviceId);
    return;
  }

  const tipoLower = tipo.toLowerCase();
  const configTopic = `grupoX/${targetDeviceId}/config`;

  const configPayload = JSON.stringify({
    comando: "ADD",
    tipo: tipoLower,
    pinos: pinos 
  });

  console.log(`[ACTION] Auto-configuring ${tipoLower} on ${targetDeviceId} pins: ${pinos.join(', ')}...`);
  client.publish(configTopic, configPayload);
  
  const payload = cmd;
  pinos.forEach(pino => {
    const topic = `grupoX/${targetDeviceId}/atuador/${tipoLower}/${pino}`;
    
    console.log(`[ACTION] Publishing to ${topic}: ${payload}`);
    client.publish(topic, String(payload));
    console.log(`Atuador acionado → ${topic}: ${payload}`);
  });
};

const state = {};

client.on('message', async (topic, msg) => {
  try {
    const payload = msg.toString();
    console.log("\nMensagem recebida:", topic, payload);

    if (topic === 'grupoX/config/response') {
      console.log("Resposta de config:", payload);
      return;
    }

    if (topic.endsWith('/status')) {
      const parts = topic.split('/');
      if (parts.length === 3 && parts[0] === 'grupoX') {
        const espId = parts[1];
        const status = payload === 'online' ? 'online' : 'offline';

        console.log(`[STATUS] ${espId} is ${status}`);

        const device = await Device.findOneAndUpdate(
          { espId },
          {
            $set: {
              status,
              lastSeen: new Date(),
              ...(payload === 'online' ? { name: `Novo ${espId}` } : {})
            },
            $setOnInsert: { components: [] } 
          },
          { upsert: true, new: true, setDefaultsOnInsert: true }
        );

        if (status === 'online') {
          console.log(`[CONFIG] Device ${espId} is online. Sending configuration...`);
          const configService = require('../services/configService');
          const payloads = configService.generateConfigPayloads(device);

          payloads.forEach(({ topic, payload }) => {
            client.publish(topic, payload);
            console.log(`[CONFIG] Sent to ${topic}: ${payload}`);
          });
        }

        return;
      }
    }

    const parts = topic.split('/');
    if (parts.length < 3) return;

    const tipoBruto = parts[2].toLowerCase();
    const modelEsperado = mapTipoToModel(tipoBruto);

    let pino = null;
    if (parts.length === 4) {
      pino = Number(parts[3]);
    } else if (parts.length >= 5) {
      // for topics like grupoX/sensor/joystick/sw25/x or grupoX/sensor/keypad/row32
      const identifier = parts[3];
      // try to extract digits; if none, fallback to NaN
      const found = identifier.match(/\d+/);
      pino = found ? Number(found[0]) : NaN;
    }
    if (isNaN(pino)) {
      console.log("Não foi possível extrair pino do tópico:", topic);
      return;
    }

    let data;
    try {
      data = JSON.parse(payload);
    } catch (e) {
      data = { valor: payload };
    }

    const device = await Device.findOne({
      components: {
        $elemMatch: {
          model: { $regex: new RegExp('^' + modelEsperado, 'i') },
          pin: { $in: [pino] } 
        }
      }
    });

    if (!device) {
      console.log(`Nenhum device com model~=${modelEsperado}, pin=${pino}`);
      return;
    }

    const espId = device.espId;

    // save reading
    try {
      await new Reading({ espId, tipo: tipoBruto, pino, data }).save();
      console.log(`Salvo para ESP ${espId} → ${tipoBruto} (${pino})`, data);

      // FIX: Update device status to online when receiving data
      await Device.findOneAndUpdate(
        { espId },
        { $set: { status: 'online', lastSeen: new Date() } }
      );
    } catch (e) {
      console.error("Erro ao salvar Reading:", e);
    }

    const rules = await Rule.find({
      deviceId: espId,
      "sensor.tipo": { $regex: new RegExp('^' + tipoBruto + '$', 'i') },
      "sensor.pino": { $in: [pino] } // Garante que o pino lido do tópico corresponda à regra
    });

    if (!rules.length) {
      console.log("Nenhuma regra para este sensor.");
      return;
    }

    console.log(`${rules.length} regras encontradas para ${tipoBruto}, pino ${pino}`);

    for (const rule of rules) {
      console.log(`Avaliando regra: ${rule.name}`);

      // get value (supports field aliases & dotted)
      let valor = extractValue(data, rule.sensor.field);

      // try secondary field if configured
      if ((valor === null || valor === undefined) && rule.sensor.field2) {
        valor = extractValue(data, rule.sensor.field2);
      }

      // temporal condition handling (optional)
      if (rule.condition.time) {
        const now = Date.now();
        if (!state[rule.name]) state[rule.name] = now;
        const diff = (now - state[rule.name]) / 1000; // seconds
        if (diff < rule.condition.time) {
          console.log(`Condição temporal ainda não satisfeita (${diff}s < ${rule.condition.time}s)`);
          continue;
        }
      }

      const ok = checkCondition(rule.condition.operator, valor, rule.condition.value, rule.condition.value2);
      if (ok) {
        console.log(`Regra '${rule.name}' acionada: valor=${valor}, operador=${rule.condition.operator}, limite=${rule.condition.value}`);
        publishAction(rule.action, rule.targetDeviceId || rule.deviceId);
      } else {
        console.log(`Condição não satisfeita para regra '${rule.name}': valor=${valor}`);
        if(rule.condition.time && state[rule.name]) {
          delete state[rule.name];
          console.log(`Contagem temporal para '${rule.name}' resetada.`);
        }
      }
    }

  } catch (err) {
    console.error("Erro no processamento MQTT:", err);
  }
});

client.on('error', (e) => console.error("Erro MQTT:", e));

module.exports = client;