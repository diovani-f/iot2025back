const mongoose = require('mongoose');

const RuleSchema = new mongoose.Schema({
  name: { type: String, required: true },
  deviceId: { type: String, required: true }, // ESP alvo
  sensor: {
    tipo: { type: String, required: true },
    pino: { type: Number, required: true }
  },
  condition: {
    operator: { type: String, enum: ['>=', '<=', '==', '>','<'], required: true },
    value: { type: Number, required: true }
  },
  action: {
    tipo: { type: String, required: true },
    pino: { type: Number, required: true },
    command: { type: String, enum: ['ON','OFF'], required: true }
  }
});

module.exports = mongoose.model('Rule', RuleSchema);
