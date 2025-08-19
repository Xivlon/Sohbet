import { Router } from 'express';
import { PostsController } from '../controllers/posts.controller';

const router = Router();
const postsController = new PostsController();

// Define routes for posts
router.get('/', postsController.getAllPosts);
router.post('/', postsController.createPost);
router.get('/:id', postsController.getPostById);
router.put('/:id', postsController.updatePost);
router.delete('/:id', postsController.deletePost);

export default router;