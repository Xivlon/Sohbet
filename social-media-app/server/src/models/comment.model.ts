import { Schema, model } from 'mongoose';

const commentSchema = new Schema({
    postId: {
        type: Schema.Types.ObjectId,
        required: true,
        ref: 'Post'
    },
    userId: {
        type: Schema.Types.ObjectId,
        required: true,
        ref: 'User'
    },
    content: {
        type: String,
        required: true,
        minlength: 1,
        maxlength: 500
    },
    createdAt: {
        type: Date,
        default: Date.now
    },
    updatedAt: {
        type: Date,
        default: Date.now
    }
});

const Comment = model('Comment', commentSchema);

export default Comment;