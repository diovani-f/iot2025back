const express = require('express');
const router = express.Router();
const Rule = require('../models/Rule');

/**
 * POST /api/rules
 * Criar nova regra de automação
 */
router.post('/', async (req, res) => {
  try {
    const { name, deviceId, sensor, condition, action, enabled, description } = req.body;

    // validação básica
    if (!name || !deviceId || !sensor || !condition || !action) {
      return res.status(400).json({
        error: 'Dados incompletos. Necessário: name, deviceId, sensor, condition, action'
      });
    }

    const rule = new Rule({
      name,
      deviceId,
      sensor: {
        tipo: sensor.tipo,
        pino: Number(sensor.pino),
        field: sensor.field
      },
      condition: {
        operator: condition.operator,
        value: condition.value,
        value2: condition.value2
      },
      action: {
        tipo: action.tipo,
        pino: Number(action.pino),
        command: action.command
      },
      enabled: enabled !== false,
      description: description || ''
    });

    await rule.save();
    console.log('✅ Regra criada:', rule);
    res.json({ message: 'Regra criada com sucesso', rule });
  } catch (err) {
    console.error('❌ Erro ao criar regra:', err);
    res.status(400).json({ error: 'Erro ao criar regra', details: err.message });
  }
});

/**
 * GET /api/rules
 * Listar todas as regras
 */
router.get('/', async (req, res) => {
  try {
    const rules = await Rule.find().sort({ name: 1 });
    res.json(rules);
  } catch (err) {
    console.error('❌ Erro ao listar regras:', err);
    res.status(500).json({ error: 'Erro ao listar regras' });
  }
});

/**
 * GET /api/rules/:id
 * Buscar uma regra específica
 */
router.get('/:id', async (req, res) => {
  try {
    const rule = await Rule.findById(req.params.id);
    if (!rule) {
      return res.status(404).json({ error: 'Regra não encontrada' });
    }
    res.json(rule);
  } catch (err) {
    console.error('❌ Erro ao buscar regra:', err);
    res.status(500).json({ error: 'Erro ao buscar regra' });
  }
});

/**
 * PUT /api/rules/:id
 * Atualizar regra existente
 */
router.put('/:id', async (req, res) => {
  try {
    const { name, deviceId, sensor, condition, action, enabled, description } = req.body;

    // validação básica
    if (!name || !deviceId || !sensor || !condition || !action) {
      return res.status(400).json({
        error: 'Dados incompletos. Necessário: name, deviceId, sensor, condition, action'
      });
    }

    const updateData = {
      name,
      deviceId,
      sensor: {
        tipo: sensor.tipo,
        pino: Number(sensor.pino),
        field: sensor.field
      },
      condition: {
        operator: condition.operator,
        value: condition.value,
        value2: condition.value2
      },
      action: {
        tipo: action.tipo,
        pino: Number(action.pino),
        command: action.command
      },
      enabled: enabled !== false,
      description: description || ''
    };

    const rule = await Rule.findByIdAndUpdate(
      req.params.id,
      updateData,
      { new: true, runValidators: true }
    );

    if (!rule) {
      return res.status(404).json({ error: 'Regra não encontrada' });
    }

    console.log('✅ Regra atualizada:', rule);
    res.json({ message: 'Regra atualizada com sucesso', rule });
  } catch (err) {
    console.error('❌ Erro ao atualizar regra:', err);
    res.status(400).json({ error: 'Erro ao atualizar regra', details: err.message });
  }
});

/**
 * DELETE /api/rules/:id
 * Deletar regra
 */
router.delete('/:id', async (req, res) => {
  try {
    const rule = await Rule.findByIdAndDelete(req.params.id);
    if (!rule) {
      return res.status(404).json({ error: 'Regra não encontrada' });
    }
    console.log('✅ Regra removida:', rule.name);
    res.json({ message: 'Regra removida com sucesso' });
  } catch (err) {
    console.error('❌ Erro ao deletar regra:', err);
    res.status(500).json({ error: 'Erro ao deletar regra' });
  }
});

module.exports = router;
