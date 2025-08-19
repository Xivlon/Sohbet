import { connect } from '../server/src/database/index';
import { User } from '../server/src/models/user.model';
import { Post } from '../server/src/models/post.model';
import { Comment } from '../server/src/models/comment.model';

const seedDatabase = async () => {
    try {
        await connect();

        // Seed Users
        const users = [
            { username: 'user1', email: 'user1@example.com', password: 'password1' },
            { username: 'user2', email: 'user2@example.com', password: 'password2' },
        ];
        await User.insertMany(users);

        // Seed Posts
        const posts = [
            { title: 'First Post', content: 'This is the content of the first post', author: users[0]._id },
            { title: 'Second Post', content: 'This is the content of the second post', author: users[1]._id },
        ];
        await Post.insertMany(posts);

        // Seed Comments
        const comments = [
            { content: 'Great post!', post: posts[0]._id, author: users[1]._id },
            { content: 'Thanks for sharing!', post: posts[1]._id, author: users[0]._id },
        ];
        await Comment.insertMany(comments);

        console.log('Database seeded successfully!');
    } catch (error) {
        console.error('Error seeding database:', error);
    } finally {
        process.exit();
    }
};

seedDatabase();