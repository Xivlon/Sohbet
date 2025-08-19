export interface Post {
    id: string;
    userId: string;
    title: string;
    content: string;
    createdAt: Date;
    updatedAt: Date;
    likes: number;
    commentsCount: number;
}

export interface CreatePostInput {
    userId: string;
    title: string;
    content: string;
}

export interface UpdatePostInput {
    title?: string;
    content?: string;
}