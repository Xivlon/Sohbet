import express from 'express';
import { json } from 'body-parser';
import { connectDatabase } from './database';
import { setupRoutes } from './routes';
import { errorMiddleware } from './middlewares/error.middleware';
import { logger } from './utils/logger';

const app = express();
const PORT = process.env.PORT || 5000;

// Middleware
app.use(json());
app.use(logger);

// Database connection
connectDatabase();

// Setup routes
setupRoutes(app);

// Error handling middleware
app.use(errorMiddleware);

// Start the server
app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});