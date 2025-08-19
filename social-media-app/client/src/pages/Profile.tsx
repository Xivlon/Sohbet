import React from 'react';
import { useAuth } from '../hooks/useAuth';

const Profile: React.FC = () => {
    const { user } = useAuth();

    return (
        <div className="profile">
            <h1>{user?.name}'s Profile</h1>
            <p>Email: {user?.email}</p>
            <p>Joined: {user?.createdAt}</p>
            {/* Additional user information can be displayed here */}
        </div>
    );
};

export default Profile;