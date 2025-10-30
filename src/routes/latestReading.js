const express = require('express');
const router = express.Router();
const Reading = require('../models/Reading');

/**
 * @swagger
 * /api/readings/{espId}/latest:
 *   get:
 *     tags: [Leituras]
 *     summary: Retorna a última leitura de um sensor
 *     parameters:
 *       - in: path
 *         name: espId
 *         required: true
 *         schema:
 *           type: string
 *         description: Identificador do sensor (ex: mpu6050_21)
 *     responses:
 *       200:
 *         description: Última leitura encontrada
 *         content:
 *           application/json:
 *             example:
 *               espId: mpu6050_21
 *               timestamp: 2025-10-30T09:00:00.000Z
 *               data: {...}
 *       404:
 *         description: Nenhuma leitura encontrada
 */
router.get('/readings/:espId/latest', async (req, res) => {
  try {
    const reading = await Reading.findOne({ espId: req.params.espId })
      .sort({ timestamp: -1 });

    if (!reading) return res.status(404).json({ error: 'Nenhuma leitura encontrada' });
    res.json(reading);
  } catch (error) {
    console.error('Erro ao buscar última leitura:', error);
    res.status(500).json({ error: 'Erro ao buscar leitura' });
  }
});

module.exports = router;
