const client = require('./client');

/**
 * Publica mensagens MQTT sem causar dependência circular.
 * @param {string} topic - Tópico do MQTT.
 * @param {string|object} payload - Dados a enviar (JSON será convertido automaticamente).
 */
function publish(topic, payload) {
  try {
    const message = typeof payload === 'object' ? JSON.stringify(payload) : payload;
    client.publish(topic, message);
    console.log(`📤 MQTT publicado em ${topic}:`, message);
  } catch (err) {
    console.error('❌ Erro ao publicar MQTT:', err);
  }
}

module.exports = { publish };
