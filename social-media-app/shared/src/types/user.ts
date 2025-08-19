export interface User {
    id: string;
    username: string;
    email: string;
    passwordHash: string;
    createdAt: Date;
    updatedAt: Date;
}

export interface UserProfile {
    userId: string;
    bio?: string;
    profilePictureUrl?: string;
    websiteUrl?: string;
    socialLinks?: {
        twitter?: string;
        facebook?: string;
        instagram?: string;
    };
}