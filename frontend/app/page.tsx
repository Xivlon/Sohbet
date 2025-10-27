"use client";
import { useState, useEffect } from 'react';
import { Sidebar } from './components/sidebar';
import { Header } from './components/header';
import { MainFeed } from './components/main-feed';
import { GlobalFeed } from './components/global-feed';
import { Khave } from './components/khave';
import { Publications } from './components/publications';
import { Muhabbet } from './components/muhabbet';
import { useIsMobile } from './components/use-mobile';

export type ActiveSection = 'main' | 'global' | 'khave' | 'publications' | 'muhabbet';

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
      default:
        return <MainFeed />;
    }
  };

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
