import { Router } from 'express';
import authRoutes from './auth.routes';
import postsRoutes from './posts.routes';
import usersRoutes from './users.routes';

const router = Router();

const setupRoutes = (app) => {
    app.use('/api/auth', authRoutes);
    app.use('/api/posts', postsRoutes);
    app.use('/api/users', usersRoutes);
};

export default setupRoutes;