import React from 'react';
import { render, screen } from '@testing-library/react';
import App from '../src/App';

describe('App Component', () => {
  test('renders the main application component', () => {
    render(<App />);
    const linkElement = screen.getByText(/welcome to the social media app/i);
    expect(linkElement).toBeInTheDocument();
  });
});