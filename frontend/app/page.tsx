"use client";
import { useState, useEffect } from 'react';
import { Sidebar } from './components/sidebar';
import { Header } from './components/header';
import { MainFeed } from './components/main-feed';
import { GlobalFeed } from './components/global-feed';
import { Khave } from './components/khave';
import { Publications } from './components/publications';
import { Muhabbet } from './components/muhabbet';
import { Profile } from './components/profile';
import { AuthModal } from './components/auth-modal';
import { useIsMobile } from './components/use-mobile';
import { useAuth } from './contexts/auth-context';

export type ActiveSection = 'main' | 'global' | 'khave' | 'publications' | 'muhabbet' | 'profile';

export default function App() {
  const [activeSection, setActiveSection] = useState<ActiveSection>('main');
  const [isDarkMode, setIsDarkMode] = useState(() => {
    // Check localStorage or system preference
    if (typeof window !== 'undefined') {
      const stored = localStorage.getItem('darkMode');
      if (stored !== null) {
        return stored === 'true';
      }
      // Check system preference
      return window.matchMedia('(prefers-color-scheme: dark)').matches;
    }
    return false;
  });
  const isMobile = useIsMobile();
  const { isAuthenticated, isLoading } = useAuth();

  useEffect(() => {
    // Apply dark mode class to document element
    if (isDarkMode) {
      document.documentElement.classList.add('dark');
    } else {
      document.documentElement.classList.remove('dark');
    }
    // Save preference
    localStorage.setItem('darkMode', String(isDarkMode));
  }, [isDarkMode]);

  const toggleDarkMode = () => {
    setIsDarkMode(!isDarkMode);
  };

  const renderActiveSection = () => {
    switch (activeSection) {
      case 'main':
        return <MainFeed />;
      case 'global':
        return <GlobalFeed />;
      case 'khave':
        return <Khave />;
      case 'publications':
        return <Publications />;
      case 'muhabbet':
        return <Muhabbet />;
      case 'profile':
        return <Profile />;
      default:
        return <MainFeed />;
    }
  };

  // Show loading state
  if (isLoading) {
    return (
      <div className="flex items-center justify-center h-screen bg-white dark:bg-gray-900">
        <div className="text-center">
          <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-red-600 dark:border-red-500 mx-auto mb-4"></div>
          <p className="text-gray-600 dark:text-gray-400">Yükleniyor...</p>
        </div>
      </div>
    );
  }

  // Show auth modal if not authenticated
  if (!isAuthenticated) {
    return (
      <div className="flex items-center justify-center h-screen bg-white dark:bg-gray-900">
        <AuthModal 
          isOpen={true} 
          onClose={() => {}} 
          initialMode="login"
          required={true}
        />
      </div>
    );
  }

  // Authenticated user - show main app
  if (isMobile) {
    return (
      <div className="flex flex-col h-screen bg-background overflow-hidden">
        <Header isDarkMode={isDarkMode} onToggleDarkMode={toggleDarkMode} />
        <main className="flex-1 overflow-hidden pb-safe">
          {renderActiveSection()}
        </main>
        <Sidebar 
          activeSection={activeSection} 
          onSectionChange={setActiveSection} 
        />
      </div>
    );
  }

  return (
    <div className="flex h-screen bg-background overflow-hidden">
      <Sidebar 
        activeSection={activeSection} 
        onSectionChange={setActiveSection} 
      />
      <div className="flex-1 flex flex-col overflow-hidden">
        <Header isDarkMode={isDarkMode} onToggleDarkMode={toggleDarkMode} />
        <main className="flex-1 overflow-hidden">
          {renderActiveSection()}
        </main>
      </div>
    </div>
  );
}
