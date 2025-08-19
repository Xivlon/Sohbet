import React from 'react';
import Feed from '../components/Feed';
import Navbar from '../components/Navbar';

const Home: React.FC = () => {
    return (
        <div>
            <Navbar />
            <h1>Welcome to the Social Media App</h1>
            <Feed />
        </div>
    );
};

export default Home;