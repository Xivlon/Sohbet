import React from 'react';
import { render, screen } from '@testing-library/react';
import PostCard from '../../src/components/PostCard';

describe('PostCard Component', () => {
    const mockPost = {
        id: 1,
        title: 'Test Post',
        content: 'This is a test post content.',
        author: 'Test Author',
        createdAt: '2023-01-01T00:00:00Z',
    };

    test('renders post title', () => {
        render(<PostCard post={mockPost} />);
        const titleElement = screen.getByText(/Test Post/i);
        expect(titleElement).toBeInTheDocument();
    });

    test('renders post content', () => {
        render(<PostCard post={mockPost} />);
        const contentElement = screen.getByText(/This is a test post content./i);
        expect(contentElement).toBeInTheDocument();
    });

    test('renders post author', () => {
        render(<PostCard post={mockPost} />);
        const authorElement = screen.getByText(/Test Author/i);
        expect(authorElement).toBeInTheDocument();
    });

    test('renders post creation date', () => {
        render(<PostCard post={mockPost} />);
        const dateElement = screen.getByText(/January 1, 2023/i);
        expect(dateElement).toBeInTheDocument();
    });
});