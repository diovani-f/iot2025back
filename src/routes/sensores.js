const express = require('express');
const router = express.Router();
const Reading = require('../models/Reading');

// GET /api/sensores - lista todos os espIds únicos
router.get('/', async (req, res) => {
  try {
    const sensores = await Reading.distinct('espId');
    res.json(sensores);
  } catch (err) {
    console.error('Erro ao listar sensores:', err);
    res.status(500).json({ error: 'Erro ao buscar sensores' });
  }
});

module.exports = router;
