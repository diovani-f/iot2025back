const mongoose = require('mongoose');

const RuleSchema = new mongoose.Schema({
  name: { type: String, required: true },

  deviceId: { type: String, required: true },

  sensor: {
    tipo: { type: String, required: true },
    pino: { type: Number, required: true },
    field: { type: String, required: true }
  },

  condition: {
    operator: {
      type: String,
      enum: ['>=', '<=', '==', '!=', '>', '<', 'between'],
      required: true
    },
    value: { type: mongoose.Schema.Types.Mixed, required: true },
    value2: { type: mongoose.Schema.Types.Mixed }, // para 'between'
    time: { type: Number } // em segundos (opcional)
  },

  targetDeviceId: { type: String, required: true }, // Device que receberá a ação

  action: {
    tipo: { type: String, required: true }, // ex: led, rele
    pino: { type: Number, required: true },
    command: { type: String, enum: ['ON', 'OFF'], required: true }
  }
});

module.exports = mongoose.model('Rule', RuleSchema);
