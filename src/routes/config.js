const express = require('express');
const router = express.Router();
const Device = require('../models/Device');
const mqttClient = require('../mqtt/client');
const configService = require('../services/configService');

router.post('/configure', async (req, res) => {
  const { name, espId, components } = req.body;

  if (!name || !espId || !Array.isArray(components)) {
    return res.status(400).json({ error: 'Dados inválidos na configuração.' });
  }

  try {
    const device = await Device.findOneAndUpdate(
      { espId },
      { name, espId, components },
      { upsert: true, new: true }
    );

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