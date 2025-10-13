const express = require('express');
const router = express.Router();
const Reading = require('../models/Reading');

/**
 * @swagger
 * /api/readings/{espId}:
 *   get:
 *     tags: [Leituras]
 *     summary: Lista as leituras recebidas de uma placa ESP32
 *     parameters:
 *       - in: path
 *         name: espId
 *         required: true
 *         schema:
 *           type: string
 *         description: Identificador da placa ESP32
 *     responses:
 *       200:
 *         description: Lista de leituras
 */
router.get('/readings/:espId', async (req, res) => {
  try {
    const readings = await Reading.find({ espId: req.params.espId }).sort({ timestamp: -1 });
    res.json(readings);
  } catch (error) {
    res.status(500).json({ error: 'Erro ao buscar leituras' });
  }
});

module.exports = router;
