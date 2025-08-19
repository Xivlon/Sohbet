# Social Media Application

This is a social media application built with a modern tech stack, including TypeScript, Express, and React. The application allows users to create accounts, post content, and interact with other users.

## Project Structure

The project is divided into three main parts:

- **Server**: The backend of the application, responsible for handling API requests, managing user authentication, and interacting with the database.
- **Client**: The frontend of the application, built with React, providing a user interface for users to interact with the application.
- **Shared**: Contains shared types and constants used across both the server and client.

## Setup Instructions

### Prerequisites

- Node.js (version 14 or higher)
- pnpm (recommended for package management)

### Installation

1. Clone the repository:

   ```
   git clone <repository-url>
   cd social-media-app
   ```

2. Install dependencies for the entire workspace:

   ```
   pnpm install
   ```

3. Set up environment variables:

   Copy the `.env.example` file to `.env` and fill in the required values.

### Running the Application

To run both the server and client in development mode, use the following command:

```
pnpm dev
```

This will start the server and client concurrently.

### Testing

To run tests for the server and client, use the following command:

```
pnpm test
```

### Build

To build the client for production, navigate to the client directory and run:

```
pnpm build
```

## Usage Guidelines

- Users can register and log in to their accounts.
- Users can create, edit, and delete posts.
- Users can view a feed of posts from other users.
- Users can interact with posts through likes and comments.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any enhancements or bug fixes.

## License

This project is licensed under the MIT License. See the LICENSE file for details.