"use client";
import { Home, Globe, Mic, BookOpen, MessageCircle, User, Users, Building2 } from 'lucide-react';
import { ActiveSection } from '../[locale]/page';
import { useIsMobile } from './use-mobile';
import { useTranslations } from 'next-intl';

interface SidebarProps {
  activeSection: ActiveSection;
  onSectionChange: (section: ActiveSection) => void;
}

const getNavigationItems = (t: any) => [
  { id: 'main' as const, label: t('home'), icon: Home, description: t('homeDescription') },
  { id: 'global' as const, label: t('global'), icon: Globe, description: t('globalDescription') },
  { id: 'groups' as const, label: t('groups'), icon: Users, description: t('groupsDescription') },
  { id: 'organizations' as const, label: t('organizations'), icon: Building2, description: t('organizationsDescription') },
  { id: 'khave' as const, label: t('khave'), icon: Mic, description: t('khaveDescription') },
  { id: 'publications' as const, label: t('publications'), icon: BookOpen, description: t('publicationsDescription') },
  { id: 'muhabbet' as const, label: t('muhabbet'), icon: MessageCircle, description: t('muhabbetDescription') },
  { id: 'profile' as const, label: t('profile'), icon: User, description: t('profileDescription') },
];

export function Sidebar({ activeSection, onSectionChange }: SidebarProps) {
  const t = useTranslations('sidebar');
  const isMobile = useIsMobile();
  const navigationItems = getNavigationItems(t);

  if (isMobile) {
    return (
      <div className="bg-white dark:bg-card border-t border-border/50 safe-area-inset-bottom">
        <nav className="flex items-center justify-evenly px-1 py-2">
          {navigationItems.map((item) => {
            const Icon = item.icon;
            const isActive = activeSection === item.id;
            
            return (
              <button
                key={item.id}
                onClick={() => onSectionChange(item.id)}
                className={`flex flex-col items-center justify-center p-2 rounded-xl transition-all duration-200 min-w-0 touch-manipulation ${
                  isActive 
                    ? 'bg-primary text-white scale-105' 
                    : 'text-muted-foreground hover:text-primary hover:bg-accent/50 active:scale-95'
                }`}
                style={{ minHeight: '60px', minWidth: '60px' }}
              >
                <Icon className={`w-6 h-6 mb-1 ${isActive ? 'text-white' : ''}`} />
                <span className={`text-xs leading-tight text-center ${isActive ? 'text-white' : ''}`}>
                  {item.label}
                </span>
              </button>
            );
          })}
        </nav>
      </div>
    );
  }

  return (
    <aside className="w-64 bg-white dark:bg-card border-r border-border/50 flex flex-col">
      <div className="p-6">
        <h2 className="text-lg font-semibold text-primary mb-1">{t('navigation')}</h2>
        <p className="text-sm text-muted-foreground">{t('departmentSwitch')}</p>
      </div>
      <nav className="flex-1 px-3 space-y-1">
        {navigationItems.map((item) => {
          const Icon = item.icon;
          const isActive = activeSection === item.id;
          
          return (
            <button
              key={item.id}
              onClick={() => onSectionChange(item.id)}
              className={`w-full flex items-center gap-3 px-4 py-3 rounded-xl transition-all duration-200 ${
                isActive 
                  ? 'bg-primary text-white' 
                  : 'text-muted-foreground hover:text-primary hover:bg-accent/50'
              }`}
            >
              <Icon className={`w-5 h-5 ${isActive ? 'text-white' : ''}`} />
              <div className="flex-1 text-left">
                <div className={`font-medium ${isActive ? 'text-white' : ''}`}>
                  {item.label}
                </div>
                <div className={`text-xs ${isActive ? 'text-white/80' : 'text-muted-foreground'}`}>
                  {item.description}
                </div>
              </div>
            </button>
          );
        })}
      </nav>
    </aside>
  );
}