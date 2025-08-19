import request from 'supertest';
import app from '../src/app'; // Adjust the path as necessary
import { createUser, deleteUser } from '../src/services/users.service'; // Adjust the path as necessary

describe('User API', () => {
  let userId;

  beforeAll(async () => {
    const user = await createUser({ username: 'testuser', password: 'password' });
    userId = user.id;
  });

  afterAll(async () => {
    await deleteUser(userId);
  });

  it('should create a new user', async () => {
    const response = await request(app)
      .post('/api/users')
      .send({ username: 'newuser', password: 'password' });

    expect(response.status).toBe(201);
    expect(response.body).toHaveProperty('id');
    expect(response.body.username).toBe('newuser');
  });

  it('should get user details', async () => {
    const response = await request(app)
      .get(`/api/users/${userId}`);

    expect(response.status).toBe(200);
    expect(response.body).toHaveProperty('id', userId);
    expect(response.body).toHaveProperty('username', 'testuser');
  });

  it('should update user details', async () => {
    const response = await request(app)
      .put(`/api/users/${userId}`)
      .send({ username: 'updateduser' });

    expect(response.status).toBe(200);
    expect(response.body).toHaveProperty('username', 'updateduser');
  });

  it('should delete a user', async () => {
    const response = await request(app)
      .delete(`/api/users/${userId}`);

    expect(response.status).toBe(204);
  });
});