require('dotenv').config();
const mongoose = require('mongoose');
const fs = require('fs');
const Rule = require('./src/models/Rule');
const Device = require('./src/models/Device');
const Reading = require('./src/models/Reading');
const client = require('./src/mqtt/client');

const originalPublish = client.publish;
client.publish = (topic, message) => {
    const log = `[MOCK MQTT] Publish to ${topic}: ${message}\n`;
    console.log(log);
    fs.appendFileSync('test_output.txt', log);
};

async function runTest() {
    try {
        await mongoose.connect(process.env.MONGO_URI || 'mongodb+srv://diovani:facco123@cluster0.mongodb.net/iot_db?retryWrites=true&w=majority');
        console.log('Connected to MongoDB');

        const triggerDevice = new Device({
            espId: 'esp32-TRIGGER',
            name: 'Trigger Device',
            components: [{ model: 'DHT11', pin: 4, type: 'sensor' }]
        });
        const targetDevice = new Device({
            espId: 'esp32-TARGET',
            name: 'Target Device',
            components: [{ model: 'LED', pin: 2, type: 'atuador' }]
        });

        await Device.deleteMany({ espId: { $in: ['esp32-TRIGGER', 'esp32-TARGET'] } });
        await Rule.deleteMany({ name: 'TEST_RULE_CROSS_DEVICE' });

        await triggerDevice.save();
        await targetDevice.save();
        console.log('Devices created');

        const rule = new Rule({
            name: 'TEST_RULE_CROSS_DEVICE',
            deviceId: 'esp32-TRIGGER',
            targetDeviceId: 'esp32-TARGET',
            sensor: { tipo: 'dht11', pino: 4, field: 'temperature' },
            condition: { operator: '>', value: 25 },
            action: { tipo: 'led', pino: 2, command: 'ON' }
        });
        await rule.save();
        console.log('Rule created');

        const topic = 'grupoX/sensor/dht11/4';
        const payload = JSON.stringify({ temperature: 30 });

        console.log(`Simulating MQTT message: ${topic} -> ${payload}`);
        client.emit('message', topic, Buffer.from(payload));

        await new Promise(resolve => setTimeout(resolve, 2000));

        console.log('Test finished. Check logs above for "[MOCK MQTT] Publish..."');

        await Device.deleteMany({ espId: { $in: ['esp32-TRIGGER', 'esp32-TARGET'] } });
        await Rule.deleteMany({ name: 'TEST_RULE_CROSS_DEVICE' });

        process.exit(0);
    } catch (err) {
        console.error('Test failed:', err);
        process.exit(1);
    }
}

runTest();
