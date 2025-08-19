// This file contains TypeScript type definitions used throughout the server. 

export interface User {
    id: string;
    username: string;
    email: string;
    password: string;
    createdAt: Date;
    updatedAt: Date;
}

export interface Post {
    id: string;
    userId: string;
    content: string;
    createdAt: Date;
    updatedAt: Date;
}

export interface Comment {
    id: string;
    postId: string;
    userId: string;
    content: string;
    createdAt: Date;
    updatedAt: Date;
}

export interface AuthResponse {
    token: string;
    user: User;
}

export interface CreatePostRequest {
    userId: string;
    content: string;
}

export interface CreateCommentRequest {
    postId: string;
    userId: string;
    content: string;
}