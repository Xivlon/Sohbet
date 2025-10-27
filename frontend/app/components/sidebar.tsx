import { Home, Globe, Mic, BookOpen, MessageCircle } from 'lucide-react';
import { ActiveSection } from '../App';

interface SidebarProps {
  activeSection: ActiveSection;
  onSectionChange: (section: ActiveSection) => void;
}

const navigationItems = [
  { id: 'main' as const, label: 'Ana Sayfa', icon: Home, description: 'Kişiselleştirilmiş akış' },
  { id: 'global' as const, label: 'Global', icon: Globe, description: 'Diğer üniversiteler' },
  { id: 'khave' as const, label: 'Khave', icon: Mic, description: 'Sesli sohbet' },
  { id: 'publications' as const, label: 'Yayınlar', icon: BookOpen, description: 'Akademik makaleler' },
  { id: 'muhabbet' as const, label: 'Muhabbet', icon: MessageCircle, description: 'Özel sohbetler' },
];

export function Sidebar({ activeSection, onSectionChange }: SidebarProps) {
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