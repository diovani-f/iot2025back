const mqtt = require('mqtt');

// Configuração do broker HiveMQ
const options = {
  host: 'wa2fc908.ala.us-east-1.emqxsl.com',
  port: 8883,
  protocol: 'mqtts',
  username: 'diovani',
  password: 'facco123'
};

const client = mqtt.connect(options);

client.on('connect', () => {
  console.log('Conectado ao broker MQTT');

  // Inscreve nos tópicos dos sensores e do botão
  client.subscribe('esp32/+/data');
  client.subscribe('grupoX/atuador/botao');
});

client.on('message', (topic, message) => {
  const payload = message.toString();

  if (topic === 'grupoX/atuador/botao') {
    console.log('Mensagem do botão recebida:', payload);
    return;
  }

  const espId = topic.split('/')[1];
  console.log(`Mensagem de sensor recebida de ${espId}:`, payload);
});

client.on('error', (err) => {
  console.error('Erro MQTT:', err);
});

module.exports = client;
