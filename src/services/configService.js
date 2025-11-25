// src/services/configService.js

/**
 * Mapeia o nome do modelo para o tipo interno usado pelo firmware/sistema.
 */
const mapModelToType = (model) => {
    const m = model.toUpperCase();
    switch (m) {
        case 'KY-023': return 'joystick_ky023';
        case 'DHT11': return 'dht11';
        case 'DHT22': return 'dht22';
        case 'DS18B20':
        case 'DS18B20SENSOR':
        case 'DS18B20_SENSOR':
        case 'DS18B20-SENSOR':
            return 'ds18b20';
        case 'HCSR04': return 'hcsr04';
        case 'MPU6050': return 'mpu6050';
        case 'APDS9960': return 'apds9960';
        case 'KEYPAD4X4': return 'keypad4x4';
        case 'BOTAO': return 'botao';
        case 'ENCODER': return 'encoder';
        case 'RELE': return 'rele';
        case 'MOTOR_VIBRACAO': return 'motor_vibracao';
        case 'LED': return 'led';
        default: return model.toLowerCase();
    }
};

/**
 * Gera os payloads de configuração para um dispositivo.
 * @param {Object} device - Objeto do dispositivo (deve conter espId e components).
 * @returns {Array} Array de objetos { topic, payload } para envio MQTT.
 */
const generateConfigPayloads = (device) => {
    if (!device || !device.espId || !Array.isArray(device.components)) {
        return [];
    }

    const grouped = {};

    device.components.forEach(c => {
        const tipo = mapModelToType(c.model);
        if (!tipo) return;

        // JOYSTICK (3 pinos)
        if (tipo === "joystick_ky023") {
            if (!grouped[tipo]) grouped[tipo] = [];
            grouped[tipo].push(c.pin);
            return;
        }

        // LED/botão/atuadores simples e sensores de 1 pino
        if (["led", "botao", "encoder", "motor_vibracao", "rele", "dht11", "dht22", "ds18b20"].includes(tipo)) {
            if (!grouped[tipo]) grouped[tipo] = [];
            grouped[tipo].push(c.pin);
            return;
        }

        // SENSORES DE DOIS PINOS
        if (["mpu6050", "apds9960", "hcsr04"].includes(tipo)) {
            if (!grouped[tipo]) grouped[tipo] = [];
            grouped[tipo].push(c.pin);
            if (c.pin_extra !== undefined) grouped[tipo].push(c.pin_extra);
            return;
        }

        // Keypad 4x4
        if (tipo === "keypad4x4") {
            if (Array.isArray(c.pinos) && c.pinos.length === 8) {
                grouped[tipo] = c.pinos;
            }
            return;
        }
    });

    const payloads = [];
    const group = process.env.GROUP || "grupoX";
    const topic = `${group}/${device.espId}/config`;

    Object.entries(grouped).forEach(([tipo, pinos]) => {
        payloads.push({
            topic,
            payload: JSON.stringify({
                comando: "ADD",
                tipo,
                pinos
            })
        });
    });

    return payloads;
};

module.exports = {
    generateConfigPayloads,
    mapModelToType
};
