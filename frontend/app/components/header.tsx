"use client";
import { Moon, Sun } from 'lucide-react';
import { Button } from './ui/button';
import { useIsMobile } from './use-mobile';

interface HeaderProps {
  isDarkMode: boolean;
  onToggleDarkMode: () => void;
}

export function Header({ isDarkMode, onToggleDarkMode }: HeaderProps) {
  const isMobile = useIsMobile();

  return (
    <header className="sticky top-0 z-50 w-full border-b bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60">
      <div className={`flex items-center justify-between ${isMobile ? 'h-14 px-4' : 'h-16 px-6'}`}>
        <div className="flex items-center gap-2">
          <h1 className={`text-primary font-semibold ${isMobile ? 'text-lg' : 'text-2xl'}`}>
            Sohbet
          </h1>
          {!isMobile && (
            <span className="text-sm text-muted-foreground ml-2">Akademik Sosyal Platform</span>
          )}
        </div>
        <Button
          variant="ghost"
          size="icon"
          onClick={onToggleDarkMode}
          className={`text-primary hover:bg-accent ${isMobile ? 'h-9 w-9' : 'h-10 w-10'}`}
          aria-label={isDarkMode ? 'Light moda geç' : 'Dark moda geç'}
        >
          {isDarkMode ? (
            <Sun className={isMobile ? 'h-5 w-5' : 'h-6 w-6'} />
          ) : (
            <Moon className={isMobile ? 'h-5 w-5' : 'h-6 w-6'} />
          )}
        </Button>
      </div>
    </header>
  );
}

