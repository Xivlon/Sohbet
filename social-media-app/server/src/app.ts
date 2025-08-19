import express from 'express';
import { json, urlencoded } from 'body-parser';
import { createServer } from 'http';
import { setupRoutes } from './routes';
import { connectDatabase } from './database';
import { errorMiddleware } from './middlewares/error.middleware';

const app = express();
const server = createServer(app);

// Middleware
app.use(json());
app.use(urlencoded({ extended: true }));

// Connect to the database
connectDatabase();

// Setup routes
setupRoutes(app);

// Error handling middleware
app.use(errorMiddleware);

const PORT = process.env.PORT || 3000;

server.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});