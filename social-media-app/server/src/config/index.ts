import dotenv from 'dotenv';

dotenv.config();

const config = {
  port: process.env.PORT || 5000,
  db: {
    uri: process.env.DB_URI || 'mongodb://localhost:27017/social-media-app',
  },
  jwt: {
    secret: process.env.JWT_SECRET || 'your_jwt_secret',
    expiresIn: process.env.JWT_EXPIRES_IN || '1h',
  },
  api: {
    baseUrl: process.env.API_BASE_URL || 'http://localhost:5000/api',
  },
};

export default config;