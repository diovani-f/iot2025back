require('dotenv').config();
const mongoose = require('mongoose');
const Device = require('./src/models/Device');
const client = require('./src/mqtt/client');

async function runTest() {
    try {
        await mongoose.connect(process.env.MONGO_URI || 'mongodb+srv://diovani:facco123@cluster0.mongodb.net/iot_db?retryWrites=true&w=majority');
        console.log('Connected to MongoDB');

        const espId = 'esp32-AUTO-DISCOVERY-TEST';
        const topic = `grupoX/${espId}/status`;
        const payload = 'online';

        console.log(`Simulating MQTT message: ${topic} -> ${payload}`);

        client.emit('message', topic, Buffer.from(payload));

        // Wait for processing
        console.log('Waiting for processing...');
        await new Promise(resolve => setTimeout(resolve, 3000));

        const device = await Device.findOne({ espId });
        if (device) {
            console.log('SUCCESS: Device created/updated via Auto-Discovery!');
            console.log('Device:', device.toObject());

            if (device.status === 'online') {
                console.log('Status is correctly set to ONLINE');
            } else {
                console.error('FAIL: Status is not online');
            }
        } else {
            console.error('FAIL: Device not found in DB');
        }

        await Device.deleteOne({ espId });
        console.log('Cleanup done');

        process.exit(0);
    } catch (err) {
        console.error('Test failed:', err);
        process.exit(1);
    }
}

runTest();
