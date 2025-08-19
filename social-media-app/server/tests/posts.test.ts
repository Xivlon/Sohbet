import request from 'supertest';
import app from '../src/app'; // Adjust the path as necessary
import { createPost, getPosts } from '../src/services/posts.service'; // Adjust the path as necessary

jest.mock('../src/services/posts.service');

describe('Posts API', () => {
  beforeEach(() => {
    jest.clearAllMocks();
  });

  describe('POST /posts', () => {
    it('should create a new post', async () => {
      const newPost = { title: 'Test Post', content: 'This is a test post.' };
      (createPost as jest.Mock).mockResolvedValue(newPost);

      const response = await request(app).post('/posts').send(newPost);

      expect(response.status).toBe(201);
      expect(response.body).toEqual(newPost);
      expect(createPost).toHaveBeenCalledWith(newPost);
    });

    it('should return 400 if post data is invalid', async () => {
      const invalidPost = { title: '', content: '' };

      const response = await request(app).post('/posts').send(invalidPost);

      expect(response.status).toBe(400);
      expect(response.body).toHaveProperty('error');
    });
  });

  describe('GET /posts', () => {
    it('should return all posts', async () => {
      const posts = [
        { title: 'Post 1', content: 'Content 1' },
        { title: 'Post 2', content: 'Content 2' },
      ];
      (getPosts as jest.Mock).mockResolvedValue(posts);

      const response = await request(app).get('/posts');

      expect(response.status).toBe(200);
      expect(response.body).toEqual(posts);
      expect(getPosts).toHaveBeenCalled();
    });
  });
});