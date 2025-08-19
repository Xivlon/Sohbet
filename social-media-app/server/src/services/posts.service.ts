import { Post } from '../models/post.model';
import { User } from '../models/user.model';

export class PostsService {
    async createPost(userId: string, content: string): Promise<Post> {
        const newPost = new Post({
            user: userId,
            content,
            createdAt: new Date(),
        });
        return await newPost.save();
    }

    async getPosts(): Promise<Post[]> {
        return await Post.find().populate('user', 'username');
    }

    async getPostById(postId: string): Promise<Post | null> {
        return await Post.findById(postId).populate('user', 'username');
    }

    async updatePost(postId: string, content: string): Promise<Post | null> {
        return await Post.findByIdAndUpdate(postId, { content }, { new: true });
    }

    async deletePost(postId: string): Promise<Post | null> {
        return await Post.findByIdAndDelete(postId);
    }

    async getUserPosts(userId: string): Promise<Post[]> {
        return await Post.find({ user: userId }).populate('user', 'username');
    }
}