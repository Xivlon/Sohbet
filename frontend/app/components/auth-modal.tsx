"use client";
import { useState } from 'react';
import { useAuth } from '../contexts/auth-context';
import { X } from 'lucide-react';
import { useTranslations } from 'next-intl';

interface AuthModalProps {
  isOpen: boolean;
  onClose: () => void;
  initialMode?: 'login' | 'register';
  required?: boolean;
}

export function AuthModal({ isOpen, onClose, initialMode = 'login', required = false }: AuthModalProps) {
  const t = useTranslations('auth');
  const tCommon = useTranslations('common');
  const [mode, setMode] = useState<'login' | 'register'>(initialMode);
  const [username, setUsername] = useState('');
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [university, setUniversity] = useState('');
  const [department, setDepartment] = useState('');
  const [error, setError] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  const [registrationSuccess, setRegistrationSuccess] = useState(false);

  const { login, register } = useAuth();

  if (!isOpen) return null;

  const handleDemoLogin = async () => {
    setError('');
    setIsLoading(true);
    setUsername('demo_student');
    setPassword('demo123');

    try {
      const result = await login({ username: 'demo_student', password: 'demo123' });
      if (result.success) {
        onClose();
        setUsername('');
        setPassword('');
      } else {
        setError(result.error || 'Demo login failed');
      }
    } finally {
      setIsLoading(false);
    }
  };

  const handleSecondDemoLogin = async () => {
    setError('');
    setIsLoading(true);
    setUsername('demo_teacher');
    setPassword('demo123');

    try {
      const result = await login({ username: 'demo_teacher', password: 'demo123' });
      if (result.success) {
        onClose();
        setUsername('');
        setPassword('');
      } else {
        setError(result.error || 'Demo login failed');
      }
    } finally {
      setIsLoading(false);
    }
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');
    setIsLoading(true);

    try {
      if (mode === 'login') {
        const result = await login({ username, password });
        if (result.success) {
          onClose();
          setUsername('');
          setPassword('');
        } else {
          setError(result.error || 'Login failed');
        }
      } else {
        const result = await register({
          username,
          email,
          password,
          university: university || undefined,
          department: department || undefined,
        });
        if (result.success) {
          setRegistrationSuccess(true);
          setUsername('');
          setEmail('');
          setPassword('');
          setUniversity('');
          setDepartment('');
        } else {
          setError(result.error || 'Registration failed');
        }
      }
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-xl max-w-md w-full p-6 relative">
        {!required && (
          <button
            onClick={onClose}
            className="absolute top-4 right-4 text-gray-500 hover:text-gray-700 dark:text-gray-400 dark:hover:text-gray-200"
          >
            <X className="w-5 h-5" />
          </button>
        )}

        <h2 className="text-2xl font-bold mb-6 text-gray-900 dark:text-white">
          {mode === 'login' ? t('login') : t('register')}
        </h2>

        {registrationSuccess && (
          <div className="mb-4 p-4 bg-green-100 dark:bg-green-900/30 text-green-700 dark:text-green-300 rounded-md">
            <div className="flex items-start">
              <svg className="w-5 h-5 mr-2 mt-0.5 flex-shrink-0" fill="currentColor" viewBox="0 0 20 20">
                <path fillRule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z" clipRule="evenodd" />
              </svg>
              <div>
                <p className="font-medium mb-1">{t('registrationSuccess')}</p>
                <p className="text-sm">{t('verifyEmailMessage')}</p>
                <button
                  onClick={onClose}
                  className="mt-3 text-sm font-medium underline hover:no-underline"
                >
                  {tCommon('close')}
                </button>
              </div>
            </div>
          </div>
        )}

        {error && (
          <div className="mb-4 p-3 bg-red-100 dark:bg-red-900/30 text-red-700 dark:text-red-300 rounded-md text-sm">
            {error}
          </div>
        )}

        {!registrationSuccess && (
          <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-1 text-gray-700 dark:text-gray-300">
              {t('username')}
            </label>
            <input
              type="text"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              required
              className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
              placeholder="kullanici_adi"
            />
          </div>

          {mode === 'register' && (
            <div>
              <label className="block text-sm font-medium mb-1 text-gray-700 dark:text-gray-300">
                {t('email')}
              </label>
              <input
                type="email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                required
                className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                placeholder="kullanici@ornek.edu"
              />
            </div>
          )}

          <div>
            <label className="block text-sm font-medium mb-1 text-gray-700 dark:text-gray-300">
              {t('password')}
            </label>
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              required
              minLength={mode === 'register' ? 8 : undefined}
              className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
              placeholder="••••••••"
            />
          </div>

          {mode === 'register' && (
            <>
              <div>
                <label className="block text-sm font-medium mb-1 text-gray-700 dark:text-gray-300">
                  {t('university')}
                </label>
                <input
                  type="text"
                  value={university}
                  onChange={(e) => setUniversity(e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                  placeholder="İstanbul Teknik Üniversitesi"
                />
              </div>

              <div>
                <label className="block text-sm font-medium mb-1 text-gray-700 dark:text-gray-300">
                  {t('department')}
                </label>
                <input
                  type="text"
                  value={department}
                  onChange={(e) => setDepartment(e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                  placeholder="Bilgisayar Mühendisliği"
                />
              </div>
            </>
          )}

          <button
            type="submit"
            disabled={isLoading}
            className="w-full bg-red-600 hover:bg-red-700 dark:bg-red-700 dark:hover:bg-red-600 text-white font-medium py-2 px-4 rounded-md transition disabled:opacity-50 disabled:cursor-not-allowed"
          >
            {isLoading ? tCommon('processing') : mode === 'login' ? t('login') : t('register')}
          </button>
        </form>
        )}

        {mode === 'login' && (
          <div className="mt-3 space-y-2">
            <button
              onClick={handleDemoLogin}
              disabled={isLoading}
              className="w-full bg-gray-100 hover:bg-gray-200 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-700 dark:text-gray-300 font-medium py-2 px-4 rounded-md transition disabled:opacity-50 disabled:cursor-not-allowed border border-gray-300 dark:border-gray-600"
            >
              🎓 Demo Hesabı 1 Kullan
            </button>
            <button
              onClick={handleSecondDemoLogin}
              disabled={isLoading}
              className="w-full bg-gray-100 hover:bg-gray-200 dark:bg-gray-700 dark:hover:bg-gray-600 text-gray-700 dark:text-gray-300 font-medium py-2 px-4 rounded-md transition disabled:opacity-50 disabled:cursor-not-allowed border border-gray-300 dark:border-gray-600"
            >
              🎓 Demo Hesabı 2 Kullan
            </button>
            <p className="text-xs text-gray-500 dark:text-gray-400 text-center mt-2">
              Uygulamayı denemek için demo hesabıyla giriş yapın
            </p>
          </div>
        )}

        <div className="mt-4 text-center">
          <button
            onClick={() => {
              setMode(mode === 'login' ? 'register' : 'login');
              setError('');
            }}
            className="text-sm text-red-600 dark:text-red-400 hover:underline"
          >
            {mode === 'login'
              ? t('dontHaveAccount')
              : t('alreadyHaveAccount')}
          </button>
        </div>
      </div>
    </div>
  );
}
