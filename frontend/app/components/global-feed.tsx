"use client";

import { useState } from 'react';
import { MapPin, TrendingUp, Users, Calendar } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Tabs, TabsContent, TabsList, TabsTrigger } from './ui/tabs';
import { Badge } from './ui/badge';

interface UniversityActivity {
  id: string;
  university: string;
  location: string;
  activity: string;
  description: string;
  timestamp: string;
  participants: number;
  category: 'research' | 'event' | 'collaboration' | 'announcement';
  trending?: boolean;
}

const mockActivities: UniversityActivity[] = [
  {
    id: '1',
    university: 'Stanford University',
    location: 'California, USA',
    activity: 'AI Research Symposium 2024',
    description: 'Yapay zeka alanında son gelişmeleri tartışan uluslararası sempozyum. 50+ ülkeden araştırmacılar katılıyor.',
    timestamp: '3 saat önce',
    participants: 1250,
    category: 'event',
    trending: true
  },
  {
    id: '2',
    university: 'ETH Zurich',
    location: 'Zurich, Switzerland',
    activity: 'Quantum Computing Breakthrough',
    description: 'Kuantum hesaplama alanında yeni bir algoritma geliştirdik. 1000 qubit\'lik sistemde test edildi.',
    timestamp: '6 saat önce',
    participants: 850,
    category: 'research',
    trending: true
  },
  {
    id: '3',
    university: 'University of Cambridge',
    location: 'Cambridge, UK',
    activity: 'Global Climate Research Initiative',
    description: 'İklim değişikliği ile mücadele için 15 üniversite ile ortak araştırma başlatıldı.',
    timestamp: '1 gün önce',
    participants: 450,
    category: 'collaboration'
  },
  {
    id: '4',
    university: 'University of Tokyo',
    location: 'Tokyo, Japan',
    activity: 'Robotics Innovation Lab Opening',
    description: 'Yeni robotik araştırma laboratuvarımız açıldı. Humanoid robotlar ve AI integration üzerine çalışacağız.',
    timestamp: '2 gün önce',
    participants: 320,
    category: 'announcement'
  },
  {
    id: '5',
    university: 'MIT',
    location: 'Massachusetts, USA',
    activity: 'Blockchain & Web3 Hackathon',
    description: '48 saatlik hackathon etkinliği. Blockchain teknolojisinde yenilikçi projeler geliştiriliyor.',
    timestamp: '3 gün önce',
    participants: 680,
    category: 'event'
  }
];

const trendingTopics = [
  'Yapay Zeka',
  'Kuantum Hesaplama',
  'İklim Araştırmaları',
  'Biyomedikal Mühendisliği',
  'Blockchain',
  'Robotik',
  'Sürdürülebilirlik',
  'Uzay Teknolojileri'
];

export function GlobalFeed() {
  const [activities] = useState<UniversityActivity[]>(mockActivities);
  const [selectedCategory, setSelectedCategory] = useState<string>('all');

  const filteredActivities = selectedCategory === 'all' 
    ? activities 
    : activities.filter(activity => activity.category === selectedCategory);

  const getCategoryColor = (category: string) => {
    switch (category) {
      case 'research': return 'bg-primary/10 text-primary border border-primary/20';
      case 'event': return 'bg-secondary text-secondary-foreground border border-border';
      case 'collaboration': return 'bg-accent text-accent-foreground border border-border';
      case 'announcement': return 'bg-muted text-muted-foreground border border-border';
      default: return 'bg-muted text-muted-foreground border border-border';
    }
  };

  const getCategoryLabel = (category: string) => {
    switch (category) {
      case 'research': return 'Araştırma';
      case 'event': return 'Etkinlik';
      case 'collaboration': return 'İşbirliği';
      case 'announcement': return 'Duyuru';
      default: return category;
    }
  };

  return (
    <div className="h-full overflow-y-auto bg-background">
      <div className="max-w-full mx-auto p-4 pb-20">
        {/* Header */}
        <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 mb-4 border-b border-border">
          <h2 className="text-primary">Global Akış</h2>
          <p className="text-muted-foreground text-sm">Dünya çapındaki üniversitelerden haberler</p>
        </div>

        <div className="flex flex-col gap-4">
          {/* Filter Tabs */}
          <Tabs defaultValue="all" onValueChange={setSelectedCategory}>
            <TabsList className="grid w-full grid-cols-5 mb-4 h-10">
              <TabsTrigger value="all" className="text-xs py-1">Tümü</TabsTrigger>
              <TabsTrigger value="research" className="text-xs py-1">Araştırma</TabsTrigger>
              <TabsTrigger value="event" className="text-xs py-1">Etkinlik</TabsTrigger>
              <TabsTrigger value="collaboration" className="text-xs py-1">İşbirliği</TabsTrigger>
              <TabsTrigger value="announcement" className="text-xs py-1">Duyuru</TabsTrigger>
            </TabsList>

              <TabsContent value={selectedCategory} className="space-y-4 mt-6">
                {filteredActivities.map((activity) => (
                  <Card key={activity.id} className="relative">
                    {activity.trending && (
                      <div className="absolute top-4 right-4">
                        <Badge variant="destructive" className="flex items-center gap-1">
                          <TrendingUp className="w-3 h-3" />
                          Trend
                        </Badge>
                      </div>
                    )}
                    
                    <CardHeader className="pb-3">
                      <div className="flex items-start justify-between">
                        <div>
                          <div className="flex items-center gap-2 mb-2">
                            <h3 className="font-semibold">{activity.university}</h3>
                            <span className={`px-2 py-1 rounded-full text-xs ${getCategoryColor(activity.category)}`}>
                              {getCategoryLabel(activity.category)}
                            </span>
                          </div>
                          <div className="flex items-center gap-2 text-muted-foreground">
                            <MapPin className="w-4 h-4" />
                            <span className="text-sm">{activity.location}</span>
                            <span className="text-sm">• {activity.timestamp}</span>
                          </div>
                        </div>
                      </div>
                    </CardHeader>
                    
                    <CardContent className="pt-0">
                      <h4 className="font-medium mb-2">{activity.activity}</h4>
                      <p className="text-muted-foreground mb-4">{activity.description}</p>
                      
                      <div className="flex items-center justify-between">
                        <div className="flex items-center gap-2 text-muted-foreground">
                          <Users className="w-4 h-4" />
                          <span className="text-sm">{activity.participants.toLocaleString()} katılımcı</span>
                        </div>
                        <div className="flex items-center gap-2">
                          <Button variant="outline" size="sm">
                            Detaylar
                          </Button>
                          <Button size="sm">
                            Takip Et
                          </Button>
                        </div>
                      </div>
                    </CardContent>
                  </Card>
                ))}
            </TabsContent>
          </Tabs>
          
          {/* Trending Topics - Mobile Optimized */}
          <Card className="mt-6">
            <CardHeader>
              <h3 className="flex items-center gap-2 text-primary">
                <TrendingUp className="w-5 h-5" />
                Trend Konular
              </h3>
            </CardHeader>
            <CardContent className="pt-0">
              <div className="flex flex-wrap gap-2">
                {trendingTopics.slice(0, 6).map((topic) => (
                  <Badge key={topic} variant="outline" className="text-xs">
                    #{topic}
                  </Badge>
                ))}
              </div>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  );
}
