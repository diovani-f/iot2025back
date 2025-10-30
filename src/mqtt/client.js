const mqtt = require('mqtt');
const Reading = require('../models/Reading');

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
  client.subscribe('grupoX/config/response');
  client.subscribe('grupoX/sensor/+/+'); // escuta todos os sensores por tipo e pino
  client.subscribe('grupoX/atuador/botao');
});

client.on('message', async (topic, message) => {
  const payload = message.toString();

  // Confirmação de configuração
  if (topic === 'grupoX/config/response') {
    console.log('Confirmação de configuração recebida:', payload);
    return;
  }

  // Botão
  if (topic === 'grupoX/atuador/botao') {
    console.log('Mensagem do botão recebida:', payload);

    const reading = new Reading({
      espId: 'botao',
      data: { estado: payload }
    });

    try {
      await reading.save();
      console.log('Evento do botão salvo no MongoDB');
    } catch (err) {
      console.error('Erro ao salvar leitura do botão:', err);
    }

    return;
  }

  // Sensores dinâmicos: grupoX/sensor/<tipo>/<pino>
  const [, , tipo, pino] = topic.split('/');

  let data;
  try {
    data = JSON.parse(payload);
  } catch {
    data = { valor: payload }; // trata como string simples
  }

  try {
    const reading = new Reading({
      espId: `${tipo}_${pino}`,
      data
    });

    await reading.save();
    console.log(`Leitura de ${tipo} no pino ${pino} salva`);
  } catch (err) {
    console.error(`Erro ao salvar leitura de ${tipo} no pino ${pino}:`, err);
  }
});

client.on('error', (err) => {
  console.error('Erro MQTT:', err);
});

module.exports = client;
