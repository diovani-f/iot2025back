const express = require('express');
const router = express.Router();
const mqttClient = require('../mqtt/client');

// POST /api/actuator
router.post('/', (req, res) => {
  const { deviceId, tipo, pin, command } = req.body;  if (!deviceId || !tipo || !pin || !command) {
    return res.status(400).json({ error: 'Dados inválidos para atuador. Faltando deviceId, tipo, pin ou command.' });
  }

  const topic = `grupoX/${deviceId}/atuador/${tipo}/${pin}`;  mqttClient.publish(topic, command);  console.log(`📡 Comando enviado via API para ${topic}: ${command}`);

  res.json({ 
    message: 'Comando enviado com sucesso via API',
    topic: topic,
    payload: command
  });
});

module.exports = router;