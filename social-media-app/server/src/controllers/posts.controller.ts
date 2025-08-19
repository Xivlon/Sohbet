import { Request, Response } from 'express';
import PostService from '../services/posts.service';

class PostsController {
    private postService: PostService;

    constructor() {
        this.postService = new PostService();
    }

    public async createPost(req: Request, res: Response): Promise<void> {
        try {
            const postData = req.body;
            const newPost = await this.postService.createPost(postData);
            res.status(201).json(newPost);
        } catch (error) {
            res.status(500).json({ message: 'Error creating post', error });
        }
    }

    public async getPosts(req: Request, res: Response): Promise<void> {
        try {
            const posts = await this.postService.getPosts();
            res.status(200).json(posts);
        } catch (error) {
            res.status(500).json({ message: 'Error fetching posts', error });
        }
    }

    public async getPostById(req: Request, res: Response): Promise<void> {
        try {
            const postId = req.params.id;
            const post = await this.postService.getPostById(postId);
            if (post) {
                res.status(200).json(post);
            } else {
                res.status(404).json({ message: 'Post not found' });
            }
        } catch (error) {
            res.status(500).json({ message: 'Error fetching post', error });
        }
    }

    public async updatePost(req: Request, res: Response): Promise<void> {
        try {
            const postId = req.params.id;
            const updatedData = req.body;
            const updatedPost = await this.postService.updatePost(postId, updatedData);
            if (updatedPost) {
                res.status(200).json(updatedPost);
            } else {
                res.status(404).json({ message: 'Post not found' });
            }
        } catch (error) {
            res.status(500).json({ message: 'Error updating post', error });
        }
    }

    public async deletePost(req: Request, res: Response): Promise<void> {
        try {
            const postId = req.params.id;
            const deletedPost = await this.postService.deletePost(postId);
            if (deletedPost) {
                res.status(204).send();
            } else {
                res.status(404).json({ message: 'Post not found' });
            }
        } catch (error) {
            res.status(500).json({ message: 'Error deleting post', error });
        }
    }
}

export default PostsController;