const express = require('express');
const router = express.Router();
const Device = require('../models/Device');
const mqttClient = require('../mqtt/client');

/**
 * @swagger
 * /api/device/{espId}/resend:
 *   post:
 *     tags: [Dispositivos]
 *     summary: Reenvia a configuração MQTT para o dispositivo
 *     parameters:
 *       - in: path
 *         name: espId
 *         required: true
 *         schema:
 *           type: string
 *         description: Identificador único da placa ESP32
 *     responses:
 *       200:
 *         description: Configuração reenviada com sucesso
 *         content:
 *           application/json:
 *             example:
 *               message: Configuração reenviada com sucesso
 *       404:
 *         description: Dispositivo não encontrado
 */
router.post('/device/:espId/resend', async (req, res) => {
  try {
    const device = await Device.findOne({ espId: req.params.espId });
    if (!device) return res.status(404).json({ error: 'Dispositivo não encontrado' });

    // Mapeamento dos modelos (igual ao config.js)
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

    // Agrupar pinos por tipo (igual ao config.js)
    const grouped = {};
    const components = device.components || [];

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

    // Enviar via MQTT no mesmo formato do configure
    const group = process.env.GROUP || "grupoX";
    const topic = `${group}/config`;

    Object.entries(grouped).forEach(([tipo, pinos]) => {
      const payload = {
        comando: "ADD",
        tipo,
        pinos
      };

      mqttClient.publish(topic, JSON.stringify(payload));
      console.log("📤 Reenviado para", topic, payload);
    });

    res.json({ message: 'Configuração reenviada com sucesso' });
  } catch (error) {
    console.error('Erro ao reenviar configuração:', error);
    res.status(500).json({ error: 'Erro ao reenviar configuração' });
  }
});

module.exports = router;
