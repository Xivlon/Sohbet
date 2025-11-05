"use client";

import { useState } from 'react';
import { Search, Download, BookOpen, Eye, Star, Filter, Calendar, User, University } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Input } from './ui/input';
import { Badge } from './ui/badge';
import { Tabs, TabsList, TabsTrigger } from './ui/tabs';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from './ui/select';

interface Publication {
  id: string;
  title: string;
  authors: string[];
  university: string;
  department: string;
  abstract: string;
  publishDate: string;
  category: string;
  tags: string[];
  downloads: number;
  views: number;
  rating: number;
  isOpenSource: boolean;
  pdfUrl?: string;
  type: 'research' | 'thesis' | 'article' | 'paper';
}

const mockPublications: Publication[] = [
  {
    id: '1',
    title: 'Yapay Zeka Algoritmalarında Etik Değerlendirme Çerçevesi',
    authors: ['Prof. Dr. Mehmet Kaya', 'Dr. Ayşe Demir', 'Ahmet Yılmaz'],
    university: 'İstanbul Teknik Üniversitesi',
    department: 'Bilgisayar Mühendisliği',
    abstract: 'Bu çalışmada, yapay zeka algoritmalarının etik değerlendirmesi için kapsamlı bir çerçeve önerilmektedir. Önerilen çerçeve, algoritmik adalet, şeffaflık ve hesap verebilirlik prensiplerine dayanmaktadır.',
    publishDate: '2024-10-01',
    category: 'Bilgisayar Bilimleri',
    tags: ['Yapay Zeka', 'Etik', 'Algoritmik Adalet', 'Makine Öğrenmesi'],
    downloads: 156,
    views: 892,
    rating: 4.8,
    isOpenSource: true,
    type: 'research'
  },
  {
    id: '2',
    title: 'Sürdürülebilir Enerji Sistemlerinde IoT Entegrasyonu',
    authors: ['Dr. Zeynep Özkan', 'Prof. Dr. Can Demir'],
    university: 'Orta Doğu Teknik Üniversitesi',
    department: 'Elektrik-Elektronik Mühendisliği',
    abstract: 'Bu araştırma, IoT teknolojilerinin sürdürülebilir enerji sistemlerine entegrasyonunu ve bu entegrasyonun enerji verimliliği üzerindeki etkilerini incelemektedir.',
    publishDate: '2024-09-15',
    category: 'Mühendislik',
    tags: ['IoT', 'Sürdürülebilir Enerji', 'Akıllı Şebeke', 'Verimlilik'],
    downloads: 89,
    views: 543,
    rating: 4.5,
    isOpenSource: true,
    type: 'paper'
  },
  {
    id: '3',
    title: 'Kuantum Hesaplama Algoritmalarının Klasik Bilgisayarlarda Simülasyonu',
    authors: ['Arş. Gör. Ali Kaya'],
    university: 'Boğaziçi Üniversitesi',
    department: 'Fizik',
    abstract: 'Bu tez çalışmasında, kuantum hesaplama algoritmalarının klasik bilgisayar ortamında simülasyonu için yeni yöntemler geliştirilerek, kuantum üstünlüğünün analizi yapılmıştır.',
    publishDate: '2024-08-22',
    category: 'Fizik',
    tags: ['Kuantum Hesaplama', 'Simülasyon', 'Algoritma', 'Kuantum Üstünlüğü'],
    downloads: 67,
    views: 321,
    rating: 4.7,
    isOpenSource: true,
    type: 'thesis'
  },
  {
    id: '4',
    title: 'Biyomedikal Sinyal İşlemede Derin Öğrenme Uygulamaları',
    authors: ['Prof. Dr. Elif Yılmaz', 'Dr. Mehmet Özdemir', 'Fatma Kaya'],
    university: 'Hacettepe Üniversitesi',
    department: 'Biyomedikal Mühendisliği',
    abstract: 'Bu çalışma, biyomedikal sinyal işlemede derin öğrenme tekniklerinin uygulanması ve bu tekniklerin geleneksel yöntemlere göre performans karşılaştırmasını sunmaktadır.',
    publishDate: '2024-07-10',
    category: 'Biyomedikal',
    tags: ['Derin Öğrenme', 'Biyomedikal', 'Sinyal İşleme', 'Sağlık Teknolojileri'],
    downloads: 234,
    views: 1245,
    rating: 4.9,
    isOpenSource: true,
    type: 'article'
  }
];

const categories = ['Tümü', 'Bilgisayar Bilimleri', 'Mühendislik', 'Fizik', 'Biyomedikal', 'Matematik', 'Kimya'];
const sortOptions = [
  { value: 'newest', label: 'En Yeni' },
  { value: 'oldest', label: 'En Eski' },
  { value: 'mostViewed', label: 'En Çok Görüntülenen' },
  { value: 'mostDownloaded', label: 'En Çok İndirilen' },
  { value: 'highestRated', label: 'En Yüksek Puanlı' }
];

export function Publications() {
  const [publications] = useState<Publication[]>(mockPublications);
  const [searchTerm, setSearchTerm] = useState('');
  const [selectedCategory, setSelectedCategory] = useState('Tümü');
  const [sortBy, setSortBy] = useState('newest');
  const [selectedType, setSelectedType] = useState<string>('all');

  const filteredPublications = publications.filter(pub => {
    const matchesSearch = pub.title.toLowerCase().includes(searchTerm.toLowerCase()) ||
                         pub.authors.some(author => author.toLowerCase().includes(searchTerm.toLowerCase())) ||
                         pub.abstract.toLowerCase().includes(searchTerm.toLowerCase()) ||
                         pub.tags.some(tag => tag.toLowerCase().includes(searchTerm.toLowerCase()));
    
    const matchesCategory = selectedCategory === 'Tümü' || pub.category === selectedCategory;
    const matchesType = selectedType === 'all' || pub.type === selectedType;
    
    return matchesSearch && matchesCategory && matchesType;
  });

  const getTypeLabel = (type: string) => {
    switch (type) {
      case 'research': return 'Araştırma';
      case 'thesis': return 'Tez';
      case 'article': return 'Makale';
      case 'paper': return 'Bildiri';
      default: return type;
    }
  };

  const getTypeColor = (type: string) => {
    switch (type) {
      case 'research': return 'bg-primary/10 text-primary border border-primary/20';
      case 'thesis': return 'bg-secondary text-secondary-foreground border border-border';
      case 'article': return 'bg-accent text-accent-foreground border border-border';
      case 'paper': return 'bg-muted text-muted-foreground border border-border';
      default: return 'bg-muted text-muted-foreground border border-border';
    }
  };

  return (
    <div className="h-full overflow-y-auto bg-background">
      <div className="max-w-full mx-auto p-4">
        {/* Header */}
        <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 mb-4 border-b border-border">
          <h2 className="text-primary">Yayınlar</h2>
          <p className="text-muted-foreground text-sm">Açık kaynak akademik makaleler ve araştırmalar</p>
        </div>

        {/* Search and Filters */}
        <div className="space-y-4 mb-6">
          <div className="flex flex-col sm:flex-row gap-4">
            <div className="flex-1 relative">
              <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-muted-foreground w-4 h-4" />
              <Input
                placeholder="Başlık, yazar, etiket veya içerik ara..."
                value={searchTerm}
                onChange={(e) => setSearchTerm(e.target.value)}
                className="pl-10"
              />
            </div>
            <Button variant="outline" className="sm:w-auto w-full">
              <Filter className="w-4 h-4 mr-2" />
              Gelişmiş Filtre
            </Button>
          </div>

          <div className="flex flex-col sm:flex-row gap-4">
            <Select value={selectedCategory} onValueChange={setSelectedCategory}>
              <SelectTrigger className="sm:w-48 w-full">
                <SelectValue placeholder="Kategori seç" />
              </SelectTrigger>
              <SelectContent>
                {categories.map(category => (
                  <SelectItem key={category} value={category}>{category}</SelectItem>
                ))}
              </SelectContent>
            </Select>

            <Select value={sortBy} onValueChange={setSortBy}>
              <SelectTrigger className="sm:w-48 w-full">
                <SelectValue placeholder="Sırala" />
              </SelectTrigger>
              <SelectContent>
                {sortOptions.map(option => (
                  <SelectItem key={option.value} value={option.value}>{option.label}</SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>
        </div>

        {/* Type Tabs */}
        <Tabs value={selectedType} onValueChange={setSelectedType} className="mb-6">
          <TabsList className="grid w-full grid-cols-5">
            <TabsTrigger value="all">Tümü</TabsTrigger>
            <TabsTrigger value="research">Araştırma</TabsTrigger>
            <TabsTrigger value="thesis">Tez</TabsTrigger>
            <TabsTrigger value="article">Makale</TabsTrigger>
            <TabsTrigger value="paper">Bildiri</TabsTrigger>
          </TabsList>
        </Tabs>

        {/* Publications Grid */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
          {filteredPublications.map((publication) => (
            <Card key={publication.id} className="hover:shadow-lg transition-shadow">
              <CardHeader className="pb-3">
                <div className="flex items-start justify-between">
                  <div className="flex-1">
                    <div className="flex items-center gap-2 mb-2">
                      <Badge className={getTypeColor(publication.type)}>
                        {getTypeLabel(publication.type)}
                      </Badge>
                      {publication.isOpenSource && (
                        <Badge variant="outline" className="text-green-600 border-green-600">
                          Açık Kaynak
                        </Badge>
                      )}
                    </div>
                    <h3 className="font-semibold mb-2 leading-tight">{publication.title}</h3>
                  </div>
                </div>
                
                <div className="space-y-1 text-sm text-muted-foreground">
                  <div className="flex items-center gap-1">
                    <User className="w-4 h-4" />
                    <span>{publication.authors.join(', ')}</span>
                  </div>
                  <div className="flex items-center gap-1">
                    <University className="w-4 h-4" />
                    <span>{publication.university} - {publication.department}</span>
                  </div>
                  <div className="flex items-center gap-1">
                    <Calendar className="w-4 h-4" />
                    <span>{new Date(publication.publishDate).toLocaleDateString('tr-TR')}</span>
                  </div>
                </div>
              </CardHeader>
              
              <CardContent className="pt-0">
                <p className="text-muted-foreground text-sm mb-4 line-clamp-3">
                  {publication.abstract}
                </p>
                
                <div className="flex flex-wrap gap-1 mb-4">
                  {publication.tags.map((tag) => (
                    <Badge key={tag} variant="secondary" className="text-xs">
                      {tag}
                    </Badge>
                  ))}
                </div>
                
                <div className="flex items-center justify-between text-sm text-muted-foreground mb-4">
                  <div className="flex items-center gap-4">
                    <div className="flex items-center gap-1">
                      <Eye className="w-4 h-4" />
                      <span>{publication.views}</span>
                    </div>
                    <div className="flex items-center gap-1">
                      <Download className="w-4 h-4" />
                      <span>{publication.downloads}</span>
                    </div>
                    <div className="flex items-center gap-1">
                      <Star className="w-4 h-4 fill-yellow-400 text-yellow-400" />
                      <span>{publication.rating}</span>
                    </div>
                  </div>
                </div>
                
                <div className="flex gap-2">
                  <Button variant="outline" size="sm" className="flex-1">
                    <BookOpen className="w-4 h-4 mr-2" />
                    Oku
                  </Button>
                  <Button size="sm" className="flex-1">
                    <Download className="w-4 h-4 mr-2" />
                    İndir
                  </Button>
                </div>
              </CardContent>
            </Card>
          ))}
        </div>

        {filteredPublications.length === 0 && (
          <div className="text-center py-12">
            <BookOpen className="w-12 h-12 text-muted-foreground mx-auto mb-4" />
            <h3 className="font-medium mb-2">Yayın bulunamadı</h3>
            <p className="text-muted-foreground">Arama kriterlerinizi değiştirmeyi deneyin.</p>
          </div>
        )}
      </div>
    </div>
  );
}
