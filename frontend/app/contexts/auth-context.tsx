"use client";
import { createContext, useContext, useState, useEffect, ReactNode } from 'react';
import { apiClient, User, LoginData, RegisterData } from '../lib/api-client';

interface AuthContextType {
  user: User | null;
  isLoading: boolean;
  isAuthenticated: boolean;
  userRole: string;
  login: (data: LoginData) => Promise<{ success: boolean; error?: string }>;
  register: (data: RegisterData) => Promise<{ success: boolean; error?: string }>;
  logout: () => void;
}

const AuthContext = createContext<AuthContextType | undefined>(undefined);

export function AuthProvider({ children }: { children: ReactNode }) {
  // Initialize state from localStorage to avoid setState in useEffect
  const [user, setUser] = useState<User | null>(() => apiClient.getUser());
  const [isLoading, setIsLoading] = useState(false);

  const login = async (data: LoginData): Promise<{ success: boolean; error?: string }> => {
    setIsLoading(true);
    console.log('Login attempt:', { username: data.username });
    const response = await apiClient.login(data);
    console.log('Login response:', { success: !response.error, status: response.status, hasData: !!response.data });
    setIsLoading(false);

    if (response.error) {
      console.error('Login error:', response.error);
      return { success: false, error: response.error };
    }

    if (response.data) {
      setUser(response.data.user);
      return { success: true };
    }

    return { success: false, error: 'Unknown error' };
  };

  const register = async (data: RegisterData): Promise<{ success: boolean; error?: string }> => {
    setIsLoading(true);
    const response = await apiClient.register(data);
    setIsLoading(false);

    if (response.error) {
      return { success: false, error: response.error };
    }

    if (response.data) {
      const loginResponse = await apiClient.login({
        username: data.username,
        password: data.password,
      });

      if (loginResponse.data) {
        setUser(loginResponse.data.user);
        apiClient.setUser(loginResponse.data.user);
        return { success: true };
      }
    }

    return { success: false, error: 'Registration succeeded but login failed' };
  };

  const logout = () => {
    apiClient.logout();
    setUser(null);
  };

  return (
    <AuthContext.Provider
      value={{
        user,
        isLoading,
        isAuthenticated: !!user,
        userRole: user?.role || 'Student',
        login,
        register,
        logout,
      }}
    >
      {children}
    </AuthContext.Provider>
  );
}

export function useAuth() {
  const context = useContext(AuthContext);
  if (context === undefined) {
    throw new Error('useAuth must be used within an AuthProvider');
  }
  return context;
}
