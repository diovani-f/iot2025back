// src/routes/configure.js

const express = require('express');
const router = express.Router();
const Device = require('../models/Device');
const mqttClient = require('../mqtt/client');
const configService = require('../services/configService');

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

    // Gerar e enviar configurações via MQTT
    const payloads = configService.generateConfigPayloads(device);

    payloads.forEach(({ topic, payload }) => {
      mqttClient.publish(topic, payload);
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