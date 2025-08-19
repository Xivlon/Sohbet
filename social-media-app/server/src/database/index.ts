import { MongoClient } from 'mongodb';
import dotenv from 'dotenv';

dotenv.config();

const uri = process.env.MONGODB_URI || '';
const client = new MongoClient(uri);

export const connectToDatabase = async () => {
    try {
        await client.connect();
        console.log('Connected to database');
    } catch (error) {
        console.error('Database connection error:', error);
        throw error;
    }
};

export const getDatabase = () => {
    return client.db(process.env.DB_NAME);
};

export const closeDatabaseConnection = async () => {
    await client.close();
    console.log('Database connection closed');
};