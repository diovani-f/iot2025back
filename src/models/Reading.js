const mongoose = require('mongoose');

const readingSchema = new mongoose.Schema({
  espId: { type: String, required: true },
  timestamp: { type: Date, default: Date.now },
  data: { type: Object, required: true }
});

module.exports = mongoose.model('Reading', readingSchema);
