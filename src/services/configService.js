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

const generateConfigPayloads = (device) => {
    if (!device || !device.espId || !Array.isArray(device.components)) {
        return [];
    }

    const grouped = {};

    device.components.forEach(c => {
        const tipo = mapModelToType(c.model);
        if (!tipo) return;

        let pins = [];
        if (Array.isArray(c.pin)) {
            pins = c.pin;
        } else if (Array.isArray(c.pinos)) {
            pins = c.pinos;
        } else if (c.pin !== undefined && c.pin !== null) {
            pins.push(c.pin);
            if (c.pin_extra !== undefined) pins.push(c.pin_extra);
        }

        if (tipo === "joystick_ky023") {
            if (pins.length > 0) {
                grouped[tipo] = pins;
            }
            return;
        }

        if (tipo === "keypad4x4") {
            if (pins.length === 8) {
                grouped[tipo] = pins;
            }
            return;
        }

        if (["mpu6050", "apds9960", "hcsr04"].includes(tipo)) {
            if (pins.length > 0) {
                grouped[tipo] = pins;
            }
            return;
        }

        if (["led", "botao", "encoder", "motor_vibracao", "rele", "dht11", "dht22", "ds18b20", "ir_receiver"].includes(tipo)) {
            if (!grouped[tipo]) grouped[tipo] = [];
            if (pins.length > 0) {
                grouped[tipo].push(pins[0]);
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
