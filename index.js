require('dotenv').config();
require('./src/mqtt/client');

const express = require('express');
const cors = require('cors');
const mongoose = require('mongoose');

const app = express();

app.use(cors());
app.use(express.json());

mongoose.connect(process.env.MONGO_URI)
  .then(() => console.log('Conectado ao MongoDB Atlas'))
  .catch(err => console.error('Erro ao conectar ao MongoDB:', err));

const swaggerUi = require('swagger-ui-express');
const swaggerSpec = require('./src/swagger');
app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerSpec));

const configRoutes = require('./src/routes/config');
const readingsRoutes = require('./src/routes/readings');
const resendRoutes = require('./src/routes/resend');
const latestReadingRoutes = require('./src/routes/latestReading');
const actuatorRoutes = require('./src/routes/actuator');
const rulesRoutes = require('./src/routes/rules');
const devicesRoutes = require('./src/routes/devices');

app.use('/api', configRoutes);
app.use('/api', readingsRoutes);
app.use('/api', resendRoutes);
app.use('/api', latestReadingRoutes);
app.use('/api/actuator', actuatorRoutes);
app.use('/api/rules', rulesRoutes);
app.use('/api', devicesRoutes);

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Servidor rodando na porta ${PORT}`);
});
