const mongoose = require('mongoose');

const ActionSchema = new mongoose.Schema({
  tipo: { type: String, required: true },
  pino: { type: mongoose.Schema.Types.Mixed, required: true },
  command: { type: String, required: true }
});

const RuleSchema = new mongoose.Schema({
  name: { type: String, required: true },

  deviceId: { type: String, required: true },

  sensor: {
    tipo: { type: String, required: true },
    pino: { type: mongoose.Schema.Types.Mixed, required: true },
    field: { type: String, required: true },
    field2: { type: String }
  },  condition: {
    operator: {
      type: String,
      // Incluídos todos os operadores suportados pelo checkCondition do client.js
      enum: ['>=', '<=', '==', '!=', '>', '<', 'between', 'in', 'notin', 'contains'], 
      required: true
    },
    value: { type: mongoose.Schema.Types.Mixed, required: true },
    value2: { type: mongoose.Schema.Types.Mixed },
    time: { type: Number }
  },  targetDeviceId: { type: String, required: true },

  action: { type: [ActionSchema], required: true }
});module.exports = mongoose.model('Rule', RuleSchema);