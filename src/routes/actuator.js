const express = require('express');
const router = express.Router();
const mqttClient = require('../mqtt/client');

// POST /api/actuator
router.post('/', (req, res) => {
  // Adicione 'deviceId' à desestruturação
  const { deviceId, tipo, pin, command } = req.body; 

  if (!deviceId || !tipo || !pin || !command) {
    // Atualize a mensagem de erro para incluir o campo faltante
    return res.status(400).json({ error: 'Dados inválidos para atuador. Faltando deviceId, tipo, pin ou command.' });
  }

  // Tópico CORRIGIDO: Inclui o ID do dispositivo (ESP ID)
  // Exemplo: grupoX/ESP4_LEDS/atuador/led/15
  const topic = `grupoX/atuador/${tipo}/${pin}`;

  // O comando agora pode ser "ON", "OFF", ou "ON_3S", etc.
  mqttClient.publish(topic, command); 
  
  console.log(`📡 Comando enviado via API para ${topic}: ${command}`);

  res.json({ 
    message: 'Comando enviado com sucesso via API',
    topic: topic,
    payload: command
  });
});

module.exports = router;