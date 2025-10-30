const express = require('express');
const router = express.Router();
const Device = require('../models/Device');
const mqttClient = require('../mqtt/client');
const { configTopic } = require('../mqtt/topics');

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

    mqttClient.publish(configTopic(device.espId), JSON.stringify({
      comando: 'CONFIG',
      espId: device.espId,
      components: device.components
    }));

    res.json({ message: 'Configuração reenviada com sucesso' });
  } catch (error) {
    console.error('Erro ao reenviar configuração:', error);
    res.status(500).json({ error: 'Erro ao reenviar configuração' });
  }
});

module.exports = router;
