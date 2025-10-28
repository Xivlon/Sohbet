import React, { useState, useEffect } from 'react';
import { View, Text, TextInput, TouchableOpacity, ScrollView, StyleSheet, ActivityIndicator } from 'react-native';
import AsyncStorage from '@react-native-async-storage/async-storage';

const API_URL = 'http://YOUR_BACKEND_URL:8080'; // Replace with your Fly.io domain

export default function App() {
  const [isLoading, setIsLoading] = useState(false);
  const [token, setToken] = useState<string | null>(null);
  const [username, setUsername] = useState('demo_student');
  const [password, setPassword] = useState('demo123');
  const [posts, setPosts] = useState<any[]>([]);
  const [error, setError] = useState('');

  // Load stored token on app start
  useEffect(() => {
    loadToken();
  }, []);

  const loadToken = async () => {
    try {
      const storedToken = await AsyncStorage.getItem('authToken');
      if (storedToken) {
        setToken(storedToken);
        fetchPosts(storedToken);
      }
    } catch (err) {
      console.error('Failed to load token:', err);
    }
  };

  const handleLogin = async () => {
    setIsLoading(true);
    setError('');
    try {
      const response = await fetch(`${API_URL}/api/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username, password }),
      });
      
      if (!response.ok) throw new Error('Login failed');
      
      const data = await response.json();
      await AsyncStorage.setItem('authToken', data.token);
      setToken(data.token);
      fetchPosts(data.token);
    } catch (err: any) {
      setError(err.message || 'Login error');
    } finally {
      setIsLoading(false);
    }
  };

  const fetchPosts = async (authToken: string) => {
    try {
      const response = await fetch(`${API_URL}/api/posts`, {
        headers: { Authorization: `Bearer ${authToken}` },
      });
      if (response.ok) {
        const data = await response.json();
        setPosts(data);
      }
    } catch (err) {
      console.error('Failed to fetch posts:', err);
    }
  };

  const handleLogout = async () => {
    await AsyncStorage.removeItem('authToken');
    setToken(null);
    setPosts([]);
  };

  if (!token) {
    return (
      <View style={styles.container}>
        <Text style={styles.title}>Sohbet - Academic Network</Text>
        {error && <Text style={styles.error}>{error}</Text>}
        
        <TextInput
          style={styles.input}
          placeholder="Username"
          value={username}
          onChangeText={setUsername}
          editable={!isLoading}
        />
        <TextInput
          style={styles.input}
          placeholder="Password"
          secureTextEntry
          value={password}
          onChangeText={setPassword}
          editable={!isLoading}
        />
        
        <TouchableOpacity
          style={[styles.button, isLoading && styles.buttonDisabled]}
          onPress={handleLogin}
          disabled={isLoading}
        >
          {isLoading ? <ActivityIndicator color="#fff" /> : <Text style={styles.buttonText}>Login</Text>}
        </TouchableOpacity>
      </View>
    );
  }

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>Sohbet Feed</Text>
        <TouchableOpacity style={styles.logoutButton} onPress={handleLogout}>
          <Text style={styles.logoutText}>Logout</Text>
        </TouchableOpacity>
      </View>

      <ScrollView style={styles.feed}>
        {posts.length === 0 ? (
          <Text style={styles.emptyText}>No posts yet. Be the first to post!</Text>
        ) : (
          posts.map((post: any) => (
            <View key={post.id} style={styles.postCard}>
              <Text style={styles.postTitle}>{post.title}</Text>
              <Text style={styles.postContent}>{post.content}</Text>
              <Text style={styles.postMeta}>{post.created_at}</Text>
            </View>
          ))
        )}
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, padding: 16, backgroundColor: '#fff' },
  title: { fontSize: 28, fontWeight: 'bold', marginBottom: 24, textAlign: 'center' },
  input: { borderWidth: 1, borderColor: '#ccc', padding: 12, marginBottom: 16, borderRadius: 8 },
  button: { backgroundColor: '#dc2626', padding: 14, borderRadius: 8, alignItems: 'center' },
  buttonDisabled: { opacity: 0.5 },
  buttonText: { color: '#fff', fontSize: 16, fontWeight: 'bold' },
  error: { color: '#dc2626', marginBottom: 16, textAlign: 'center' },
  header: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', marginBottom: 16 },
  logoutButton: { padding: 8 },
  logoutText: { color: '#dc2626', fontWeight: 'bold' },
  feed: { flex: 1 },
  emptyText: { textAlign: 'center', color: '#999', marginTop: 32 },
  postCard: { borderWidth: 1, borderColor: '#e5e7eb', padding: 12, marginBottom: 12, borderRadius: 8 },
  postTitle: { fontSize: 16, fontWeight: 'bold', marginBottom: 4 },
  postContent: { fontSize: 14, color: '#333', marginBottom: 8 },
  postMeta: { fontSize: 12, color: '#999' },
});