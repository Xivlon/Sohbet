"use client";

import { useLocale } from 'next-intl';
import { usePathname, useRouter } from 'next/navigation';
import { Globe } from 'lucide-react';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from './ui/dropdown-menu';
import { Button } from './ui/button';

const languages = [
  { code: 'tr', name: 'Türkçe', flag: '🇹🇷' },
  { code: 'en', name: 'English', flag: '🇬🇧' }
];

export function LanguageSwitcher() {
  const locale = useLocale();
  const router = useRouter();
  const pathname = usePathname();

  const switchLocale = (newLocale: string) => {
    // Replace the locale in the pathname
    const newPathname = pathname.replace(`/${locale}`, `/${newLocale}`);
    router.push(newPathname);
  };

  const currentLanguage = languages.find(lang => lang.code === locale);

  return (
    <DropdownMenu>
      <DropdownMenuTrigger asChild>
        <Button
          variant="ghost"
          size="icon"
          className="relative"
          aria-label="Change language"
        >
          <Globe className="h-5 w-5" />
          <span className="sr-only">Change language</span>
        </Button>
      </DropdownMenuTrigger>
      <DropdownMenuContent align="end">
        {languages.map((language) => (
          <DropdownMenuItem
            key={language.code}
            onClick={() => switchLocale(language.code)}
            className={locale === language.code ? 'bg-accent' : ''}
          >
            <span className="mr-2">{language.flag}</span>
            {language.name}
            {locale === language.code && (
              <span className="ml-auto text-xs">✓</span>
            )}
          </DropdownMenuItem>
        ))}
      </DropdownMenuContent>
    </DropdownMenu>
  );
}
