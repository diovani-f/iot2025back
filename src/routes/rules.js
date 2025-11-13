const express = require('express');
const router = express.Router();
const Rule = require('../models/Rule');

// Criar regra
router.post('/', async (req, res) => {
  try {
    const rule = new Rule(req.body);
    await rule.save();
    res.json({ message: 'Regra criada com sucesso', rule });
  } catch (err) {
    res.status(400).json({ error: 'Erro ao criar regra', details: err });
  }
});

// Listar regras
router.get('/', async (req, res) => {
  const rules = await Rule.find();
  res.json(rules);
});

// Deletar regra
router.delete('/:id', async (req, res) => {
  await Rule.findByIdAndDelete(req.params.id);
  res.json({ message: 'Regra removida com sucesso' });
});

module.exports = router;
