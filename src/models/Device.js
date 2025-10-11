const mongoose = require('mongoose');

const componentSchema = new mongoose.Schema({
  name: String,         // Nome mais de boa (ex: "Sensor de Temperatura")
  model: String,        // Modelo técnico (ex: "DS18B20")
  type: String,         // "sensor" ou "atuador"
  pin: Number,          // Pino conectado
  interval: Number,     // Para sensores: tempo de leitura (ms)
  unit: String,         // Unidade de medida (ex: °C, ppm)
  label: String,        // Nome para exibição
  config: Object        // Configurações específicas
});

const deviceSchema = new mongoose.Schema({
  name: String,         // Nome da placa
  espId: String,        // ID único da placa
  components: [componentSchema]
});

module.exports = mongoose.model('Device', deviceSchema);
