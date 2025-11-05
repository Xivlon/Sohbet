"use client";
import { useState } from 'react';
import { Moon, Sun, LogIn, LogOut, User } from 'lucide-react';
import { Button } from './ui/button';
import { useIsMobile } from './use-mobile';
import { useAuth } from '../contexts/auth-context';
import { AuthModal } from './auth-modal';
import { LanguageSwitcher } from './language-switcher';
import { useTranslations } from 'next-intl';

interface HeaderProps {
  isDarkMode: boolean;
  onToggleDarkMode: () => void;
}

export function Header({ isDarkMode, onToggleDarkMode }: HeaderProps) {
  const t = useTranslations('header');
  const isMobile = useIsMobile();
  const { user, isAuthenticated, logout } = useAuth();
  const [showAuthModal, setShowAuthModal] = useState(false);

  return (
    <>
      <header className="sticky top-0 z-50 w-full border-b bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60">
        <div className={`flex items-center justify-between ${isMobile ? 'h-14 px-4' : 'h-16 px-6'}`}>
          <div className="flex items-center gap-2">
            <h1 className={`text-primary font-semibold ${isMobile ? 'text-lg' : 'text-2xl'}`}>
              {t('title')}
            </h1>
            {!isMobile && (
              <span className="text-sm text-muted-foreground ml-2">{t('subtitle')}</span>
            )}
          </div>
          
          <div className="flex items-center gap-2">
            <LanguageSwitcher />
            {isAuthenticated && user ? (
              <>
                {!isMobile && (
                  <div className="flex items-center gap-2 mr-2">
                    <User className="h-4 w-4 text-muted-foreground" />
                    <span className="text-sm text-foreground">{user.username}</span>
                  </div>
                )}
                <Button
                  variant="ghost"
                  size={isMobile ? "sm" : "default"}
                  onClick={logout}
                  className="text-primary hover:bg-accent"
                >
                  <LogOut className={isMobile ? 'h-4 w-4 mr-1' : 'h-5 w-5 mr-2'} />
                  {isMobile ? '' : t('logout')}
                </Button>
              </>
            ) : (
              <Button
                variant="ghost"
                size={isMobile ? "sm" : "default"}
                onClick={() => setShowAuthModal(true)}
                className="text-primary hover:bg-accent"
              >
                <LogIn className={isMobile ? 'h-4 w-4 mr-1' : 'h-5 w-5 mr-2'} />
                {isMobile ? '' : t('login')}
              </Button>
            )}
            
            <Button
              variant="ghost"
              size="icon"
              onClick={onToggleDarkMode}
              className={`text-primary hover:bg-accent ${isMobile ? 'h-9 w-9' : 'h-10 w-10'}`}
              aria-label={isDarkMode ? t('lightMode') : t('darkMode')}
            >
              {isDarkMode ? (
                <Sun className={isMobile ? 'h-5 w-5' : 'h-6 w-6'} />
              ) : (
                <Moon className={isMobile ? 'h-5 w-5' : 'h-6 w-6'} />
              )}
            </Button>
          </div>
        </div>
      </header>

      <AuthModal 
        isOpen={showAuthModal} 
        onClose={() => setShowAuthModal(false)}
      />
    </>
  );
}

