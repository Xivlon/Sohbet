import { API_URL } from '../../app/lib/config';

const envBase = typeof process !== 'undefined' ? API_URL || '' : '';
const API_BASE = envBase.replace(/\/+$/, ''); // remove trailing slashes

export type LoginResult = { token: string; user: unknown };

async function request(path: string, options: RequestInit = {}) {
  // Use configured API base URL, do not fallback to window.location
  if (!API_BASE) {
    throw new Error('API base URL not configured. Set NEXT_PUBLIC_API_URL environment variable.');
  }
  
  const url = `${API_BASE}${path.startsWith('/') ? path : `/${path}`}`;
  const res = await fetch(url, options);
  return res;
}

export async function login(username: string, password: string): Promise<LoginResult> {
  const res = await request('/api/login', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ username, password }),
  });

  if (!res.ok) {
    const text = await res.text().catch(() => '');
    let msg = `Login failed: ${res.status}`;
    try {
      const parsed = JSON.parse(text);
      msg += ` - ${parsed.error || JSON.stringify(parsed)}`;
    } catch {
      msg += ` - ${text}`;
    }
    throw new Error(msg);
  }

  const data = await res.json();
  if (!data || !data.token) throw new Error('Login response did not include a token.');
  try {
    localStorage.setItem('auth_token', data.token);
  } catch {
    console.warn('Could not store token');
  }
  return { token: data.token, user: data.user };
}

export function logout() {
  try {
    localStorage.removeItem('auth_token');
  } catch {
    /* ignore */
  }
}
