import React from 'react';

interface PostCardProps {
    title: string;
    content: string;
    author: string;
    createdAt: string;
}

const PostCard: React.FC<PostCardProps> = ({ title, content, author, createdAt }) => {
    return (
        <div className="post-card">
            <h2>{title}</h2>
            <p>{content}</p>
            <p className="post-author">By: {author}</p>
            <p className="post-date">{new Date(createdAt).toLocaleDateString()}</p>
        </div>
    );
};

export default PostCard;