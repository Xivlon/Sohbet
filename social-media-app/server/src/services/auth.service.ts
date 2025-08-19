import { User } from '../models/user.model';
import bcrypt from 'bcrypt';
import jwt from 'jsonwebtoken';

export class AuthService {
    private jwtSecret: string;

    constructor() {
        this.jwtSecret = process.env.JWT_SECRET || 'your_jwt_secret';
    }

    async register(userData: { username: string; password: string }): Promise<User> {
        const hashedPassword = await bcrypt.hash(userData.password, 10);
        const user = new User({ ...userData, password: hashedPassword });
        await user.save();
        return user;
    }

    async login(username: string, password: string): Promise<string | null> {
        const user = await User.findOne({ username });
        if (user && await bcrypt.compare(password, user.password)) {
            return this.generateToken(user._id);
        }
        return null;
    }

    private generateToken(userId: string): string {
        return jwt.sign({ id: userId }, this.jwtSecret, { expiresIn: '1h' });
    }

    async validateToken(token: string): Promise<any> {
        try {
            return jwt.verify(token, this.jwtSecret);
        } catch (error) {
            return null;
        }
    }
}