const express = require('express');
const router = express.Router();
const Device = require('../models/Device');
const mqttClient = require('../mqtt/client');
const { configTopic } = require('../mqtt/topics');

/**
 * @swagger
 * /api/configure:
 *   post:
 *     tags: [Dispositivos]
 *     summary: Cria ou atualiza a configuração de uma placa ESP32
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/Device'
 *           example:
 *             name: ESP32 Sala
 *             espId: esp32_sala_01
 *             components:
 *               - name: Sensor de Temperatura
 *                 model: DS18B20
 *                 type: sensor
 *                 pin: 4
 *                 interval: 10000
 *                 unit: °C
 *                 label: Temperatura ambiente
 *                 config: {}
 *     responses:
 *       200:
 *         description: Configuração salva com sucesso
 *         content:
 *           application/json:
 *             example:
 *               message: Configuração salva com sucesso
 *               device:
 *                 name: ESP32 Sala
 *                 espId: esp32_sala_01
 *                 components: [...]
 */
router.post('/configure', async (req, res) => {
  const { name, espId, components } = req.body;

  try {
    const device = await Device.findOneAndUpdate(
      { espId },
      { name, components },
      { upsert: true, new: true }
    );

    // Publica a configuração no tópico MQTT específico do ESP32
    mqttClient.publish(configTopic(espId), JSON.stringify({
      comando: 'CONFIG',
      espId,
      components
    }));

    res.json({ message: 'Configuração salva com sucesso', device });
  } catch (error) {
    console.error('Erro ao salvar configuração:', error);
    res.status(500).json({ error: 'Erro ao salvar configuração' });
  }
});

/**
 * @swagger
 * /api/devices:
 *   get:
 *     tags: [Dispositivos]
 *     summary: Lista todos os dispositivos cadastrados
 *     responses:
 *       200:
 *         description: Lista de dispositivos
 *         content:
 *           application/json:
 *             example:
 *               - name: ESP32 Sala
 *                 espId: esp32_sala_01
 *                 components: [...]
 */
router.get('/devices', async (req, res) => {
  try {
    const devices = await Device.find();
    res.json(devices);
  } catch (error) {
    res.status(500).json({ error: 'Erro ao buscar dispositivos' });
  }
});

/**
 * @swagger
 * /api/device/{espId}:
 *   get:
 *     tags: [Dispositivos]
 *     summary: Busca um dispositivo pelo espId
 *     parameters:
 *       - in: path
 *         name: espId
 *         required: true
 *         schema:
 *           type: string
 *         description: Identificador único da placa ESP32
 *     responses:
 *       200:
 *         description: Dispositivo encontrado
 *         content:
 *           application/json:
 *             example:
 *               name: ESP32 Sala
 *               espId: esp32_sala_01
 *               components: [...]
 *       404:
 *         description: Dispositivo não encontrado
 */
router.get('/device/:espId', async (req, res) => {
  try {
    const device = await Device.findOne({ espId: req.params.espId });
    if (!device) return res.status(404).json({ error: 'Dispositivo não encontrado' });
    res.json(device);
  } catch (error) {
    res.status(500).json({ error: 'Erro ao buscar dispositivo' });
  }
});

/**
 * @swagger
 * /api/device/{espId}:
 *   put:
 *     tags: [Dispositivos]
 *     summary: Atualiza um dispositivo existente
 *     parameters:
 *       - in: path
 *         name: espId
 *         required: true
 *         schema:
 *           type: string
 *         description: Identificador único da placa ESP32
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/Device'
 *           example:
 *             name: ESP32 Sala Atualizada
 *             components:
 *               - name: Sensor de Gestos
 *                 model: APDS-9960
 *                 type: sensor
 *                 pin: 14
 *                 interval: 8000
 *                 unit: gesto
 *                 label: Controle por gestos
 *                 config: {}
 *     responses:
 *       200:
 *         description: Dispositivo atualizado com sucesso
 *         content:
 *           application/json:
 *             example:
 *               message: Dispositivo atualizado com sucesso
 *               device: {...}
 *       404:
 *         description: Dispositivo não encontrado
 */
router.put('/device/:espId', async (req, res) => {
  const { name, components } = req.body;

  try {
    const updated = await Device.findOneAndUpdate(
      { espId: req.params.espId },
      { name, components },
      { new: true }
    );

    if (!updated) return res.status(404).json({ error: 'Dispositivo não encontrado' });
    res.json({ message: 'Dispositivo atualizado com sucesso', device: updated });
  } catch (error) {
    res.status(500).json({ error: 'Erro ao atualizar dispositivo' });
  }
});

/**
 * @swagger
 * /api/device/{espId}:
 *   delete:
 *     tags: [Dispositivos]
 *     summary: Remove um dispositivo pelo espId
 *     parameters:
 *       - in: path
 *         name: espId
 *         required: true
 *         schema:
 *           type: string
 *         description: Identificador único da placa ESP32
 *     responses:
 *       200:
 *         description: Dispositivo removido com sucesso
 *         content:
 *           application/json:
 *             example:
 *               message: Dispositivo removido com sucesso
 *       404:
 *         description: Dispositivo não encontrado
 */
router.delete('/device/:espId', async (req, res) => {
  try {
    const deleted = await Device.findOneAndDelete({ espId: req.params.espId });
    if (!deleted) return res.status(404).json({ error: 'Dispositivo não encontrado' });
    res.json({ message: 'Dispositivo removido com sucesso' });
  } catch (error) {
    res.status(500).json({ error: 'Erro ao remover dispositivo' });
  }
});

module.exports = router;
