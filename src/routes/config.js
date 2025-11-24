// src/routes/configure.js

const express = require('express');
const router = express.Router();
const Device = require('../models/Device');
const mqttClient = require('../mqtt/client');

/**
 * POST /api/configure
 * Salva a configuração e envia para o ESP no formato correto
 */
router.post('/configure', async (req, res) => {
  const { name, espId, components } = req.body;

  if (!name || !espId || !Array.isArray(components)) {
    return res.status(400).json({ error: 'Dados inválidos na configuração.' });
  }

  try {
    // Salvar no banco
    const device = await Device.findOneAndUpdate(
      { espId },
      { name, espId, components },
      { upsert: true, new: true }
    );

    // Mapeamento dos modelos
    const tipoMapeado = (model) => {
      const m = model.toUpperCase();
      switch (m) {
        case 'KY-023':
          return 'joystick_ky023';

        case 'DHT11':
          return 'dht11';

        case 'DHT22':
          return 'dht22';

        case 'DS18B20':
        case 'DS18B20SENSOR':
        case 'DS18B20_SENSOR':
        case 'DS18B20-SENSOR':
          return 'ds18b20';

        case 'HCSR04':
          return 'hcsr04';

        case 'MPU6050':
          return 'mpu6050';

        case 'APDS9960':
          return 'apds9960';

        case 'KEYPAD4X4':
          return 'keypad4x4';

        case 'BOTAO':
          return 'botao';

        case 'ENCODER':
          return 'encoder';

        case 'RELE':
          return 'rele';

        case 'MOTOR_VIBRACAO':
          return 'motor_vibracao';

        case 'LED':
          return 'led';

        default:
          return model.toLowerCase();
      }
    };


    // Agrupar pinos por tipo
    const grouped = {};

    components.forEach(c => {
      const tipo = tipoMapeado(c.model);
      if (!tipo) return;

      // JOYSTICK (3 pinos)
      if (tipo === "joystick_ky023") {
        if (!grouped[tipo]) grouped[tipo] = [];
        grouped[tipo].push(c.pin);
        return;
      }

      // LED/botão/atuadores simples
      if (["led", "botao", "encoder", "motor_vibracao", "rele", "dht11", "dht22", "ds18b20"].includes(tipo)) {
        if (!grouped[tipo]) grouped[tipo] = [];
        grouped[tipo].push(c.pin);
        return;
      }

      // SENSORES DE DOIS PINOS
      if (["mpu6050", "apds9960", "hcsr04"].includes(tipo)) {
        if (!grouped[tipo]) grouped[tipo] = [];
        grouped[tipo].push(c.pin);
        if (c.pin_extra !== undefined) grouped[tipo].push(c.pin_extra);
        return;
      }

      // Keypad 4x4
      if (tipo === "keypad4x4") {
        if (Array.isArray(c.pinos) && c.pinos.length === 8) {
          grouped[tipo] = c.pinos;
        }
        return;
      }
    });

    // Enviar via MQTT
    const group = process.env.GROUP || "grupoX";
    const topic = `${group}/${espId}/config`;

    Object.entries(grouped).forEach(([tipo, pinos]) => {
      const payload = {
        comando: "ADD",
        tipo,
        pinos
      };

      mqttClient.publish(topic, JSON.stringify(payload));
      console.log("📤 Enviado para", topic, payload);
    });

    return res.json({
      message: "Configuração salva e enviada com sucesso",
      device
    });

  } catch (err) {
    console.error("Erro ao salvar configuração:", err);
    return res.status(500).json({ error: "Erro interno ao salvar configuração." });
  }
});

module.exports = router;