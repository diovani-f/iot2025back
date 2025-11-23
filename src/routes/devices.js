const express = require('express');
const router = express.Router();
const Device = require('../models/Device');

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
 *             schema:
 *               type: array
 *               items:
 *                 type: object
 *                 properties:
 *                   _id:
 *                     type: string
 *                   name:
 *                     type: string
 *                   espId:
 *                     type: string
 *                   components:
 *                     type: array
 */
router.get('/devices', async (req, res) => {
    try {
        const devices = await Device.find().sort({ name: 1 });
        res.json(devices);
    } catch (error) {
        console.error('Erro ao buscar dispositivos:', error);
        res.status(500).json({ error: 'Erro ao buscar dispositivos' });
    }
});

module.exports = router;
