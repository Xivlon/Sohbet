// Helper functions for API calls in new components
// These wrap fetch with proper authentication

function getAuthHeaders(): HeadersInit {
  const token = typeof window !== 'undefined' ? localStorage.getItem('auth_token') : null
  return {
    'Content-Type': 'application/json',
    ...(token ? { 'Authorization': `Bearer ${token}` } : {})
  }
}

export const api = {
  async get(endpoint: string) {
    const baseUrl = process.env.NEXT_PUBLIC_API_URL || 'https://sohbet-uezxqq.fly.dev'
    return fetch(`${baseUrl}${endpoint}`, {
      headers: getAuthHeaders()
    })
  },

  async post(endpoint: string, body?: any) {
    const baseUrl = process.env.NEXT_PUBLIC_API_URL || 'https://sohbet-uezxqq.fly.dev'
    return fetch(`${baseUrl}${endpoint}`, {
      method: 'POST',
      headers: getAuthHeaders(),
      body: body ? JSON.stringify(body) : undefined
    })
  },

  async put(endpoint: string, body?: any) {
    const baseUrl = process.env.NEXT_PUBLIC_API_URL || 'https://sohbet-uezxqq.fly.dev'
    return fetch(`${baseUrl}${endpoint}`, {
      method: 'PUT',
      headers: getAuthHeaders(),
      body: body ? JSON.stringify(body) : undefined
    })
  },

  async delete(endpoint: string) {
    const baseUrl = process.env.NEXT_PUBLIC_API_URL || 'https://sohbet-uezxqq.fly.dev'
    return fetch(`${baseUrl}${endpoint}`, {
      method: 'DELETE',
      headers: getAuthHeaders()
    })
  }
}
