// src/index.js
require('./src/mqtt/client');
require('dotenv').config();
const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const mongoose = require('mongoose');


// Inicializa o app
const app = express();
app.use(cors());
app.use(bodyParser.json());

// Conexão com MongoDB Atlas
mongoose.connect(process.env.MONGO_URI)
  .then(() => console.log('Conectado ao MongoDB Atlas'))
  .catch(err => console.error('Erro ao conectar ao MongoDB:', err));

const swaggerUi = require('swagger-ui-express');
const swaggerSpec = require('./src/swagger');

app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerSpec));

const configRoutes = require('./src/routes/config');
app.use('/api', configRoutes);

const readingsRoutes = require('./src/routes/readings');
app.use('/api', readingsRoutes);

// Inicializa o servidor
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Servidor rodando na porta ${PORT}`);
});
