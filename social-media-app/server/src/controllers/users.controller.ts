import { Request, Response } from 'express';
import UserService from '../services/users.service';

class UsersController {
    private userService: UserService;

    constructor() {
        this.userService = new UserService();
    }

    public async getAllUsers(req: Request, res: Response): Promise<Response> {
        try {
            const users = await this.userService.findAll();
            return res.status(200).json(users);
        } catch (error) {
            return res.status(500).json({ message: 'Error retrieving users' });
        }
    }

    public async getUserById(req: Request, res: Response): Promise<Response> {
        const { id } = req.params;
        try {
            const user = await this.userService.findById(id);
            if (!user) {
                return res.status(404).json({ message: 'User not found' });
            }
            return res.status(200).json(user);
        } catch (error) {
            return res.status(500).json({ message: 'Error retrieving user' });
        }
    }

    public async createUser(req: Request, res: Response): Promise<Response> {
        const userData = req.body;
        try {
            const newUser = await this.userService.create(userData);
            return res.status(201).json(newUser);
        } catch (error) {
            return res.status(500).json({ message: 'Error creating user' });
        }
    }

    public async updateUser(req: Request, res: Response): Promise<Response> {
        const { id } = req.params;
        const userData = req.body;
        try {
            const updatedUser = await this.userService.update(id, userData);
            if (!updatedUser) {
                return res.status(404).json({ message: 'User not found' });
            }
            return res.status(200).json(updatedUser);
        } catch (error) {
            return res.status(500).json({ message: 'Error updating user' });
        }
    }

    public async deleteUser(req: Request, res: Response): Promise<Response> {
        const { id } = req.params;
        try {
            const deletedUser = await this.userService.delete(id);
            if (!deletedUser) {
                return res.status(404).json({ message: 'User not found' });
            }
            return res.status(204).send();
        } catch (error) {
            return res.status(500).json({ message: 'Error deleting user' });
        }
    }
}

export default new UsersController();