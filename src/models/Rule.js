const mongoose = require('mongoose');

// Define um sub-schema para uma única Ação de Atuador
const ActionSchema = new mongoose.Schema({
  tipo: { type: String, required: true }, // ex: 'led', 'rele', 'motor_vibracao'
  pino: { type: mongoose.Schema.Types.Mixed, required: true },
  command: { type: String, required: true } 
});

const RuleSchema = new mongoose.Schema({
  name: { type: String, required: true },

  deviceId: { type: String, required: true }, // ESP que envia o dado

  sensor: {
    tipo: { type: String, required: true },
    pino: { type: mongoose.Schema.Types.Mixed, required: true },
    field: { type: String, required: true },
    field2: { type: String } // Adicionado para suportar a lógica field2 do client.js
  },

  condition: {
    operator: {
      type: String,
      // Incluídos todos os operadores suportados pelo checkCondition do client.js
      enum: ['>=', '<=', '==', '!=', '>', '<', 'between', 'in', 'notin', 'contains'], 
      required: true
    },
    value: { type: mongoose.Schema.Types.Mixed, required: true },
    value2: { type: mongoose.Schema.Types.Mixed }, 
    time: { type: Number } // em segundos (para regras como Porta Aberta > 5s)
  },

  targetDeviceId: { type: String, required: true }, // ESP que recebe os comandos (pode ser o mesmo ou outro)

  // ALTERAÇÃO PRINCIPAL: Permite um array de Ações (para acionamento em cascata)
  action: { type: [ActionSchema], required: true } 
});

module.exports = mongoose.model('Rule', RuleSchema);