const envBase = typeof process !== 'undefined' ? process.env.NEXT_PUBLIC_API_URL || '' : '';
const API_BASE = envBase.replace(/\/+$/, ''); // remove trailing slashes

export type LoginResult = { token: string; user: unknown };

async function request(path: string, options: RequestInit = {}) {
  const base =
    API_BASE ||
    (typeof window !== 'undefined' ? `${window.location.protocol}//${window.location.host}` : '');
  if (!base) throw new Error('API base URL not configured (NEXT_PUBLIC_API_URL)');
  const url = `${base}${path.startsWith('/') ? path : `/${path}`}`;
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
    localStorage.setItem('authToken', data.token);
  } catch {
    console.warn('Could not store token');
  }
  return { token: data.token, user: data.user };
}

export function logout() {
  try {
    localStorage.removeItem('authToken');
  } catch {
    /* ignore */
  }
}
