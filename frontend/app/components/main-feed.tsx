"use client";

import { useState } from 'react';
import { Heart, MessageCircle, Share, MoreHorizontal, Plus } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Textarea } from './ui/textarea';

interface Post {
  id: string;
  author: {
    name: string;
    department: string;
    university: string;
    avatar: string;
  };
  content: string;
  timestamp: string;
  likes: number;
  comments: number;
  shares: number;
  isLiked: boolean;
  type: 'announcement' | 'discussion' | 'event' | 'academic';
}

const mockPosts: Post[] = [
  {
    id: '1',
    author: {
      name: 'Prof. Dr. Mehmet Kaya',
      department: 'Bilgisayar Mühendisliği',
      university: 'İTÜ',
      avatar: 'MK'
    },
    content: 'Yeni araştırma projemiz "Yapay Zeka ve Etik" konusunda başladı. Bu konuda çalışmak isteyen öğrenciler bana ulaşabilir. Proje kapsamında makine öğrenmesi, doğal dil işleme ve etik algoritmalar üzerine çalışacağız.',
    timestamp: '2 saat önce',
    likes: 24,
    comments: 8,
    shares: 3,
    isLiked: false,
    type: 'academic'
  },
  {
    id: '2',
    author: {
      name: 'Ayşe Demir',
      department: 'Endüstri Mühendisliği',
      university: 'ODTÜ',
      avatar: 'AD'
    },
    content: 'Yarın saat 14:00\'da "Sürdürülebilir Üretim Teknikleri" konulu seminerimiz var. Herkes davetli! Konferans salonunda olacak.',
    timestamp: '5 saat önce',
    likes: 15,
    comments: 3,
    shares: 7,
    isLiked: true,
    type: 'event'
  },
  {
    id: '3',
    author: {
      name: 'Dr. Can Özkan',
      department: 'Fizik',
      university: 'Boğaziçi',
      avatar: 'CÖ'
    },
    content: 'Kuantum hesaplama alanında yaptığımız son çalışma Journal of Quantum Computing\'de yayınlandı. Makaleye erişim için profile bakabilirsiniz.',
    timestamp: '1 gün önce',
    likes: 42,
    comments: 12,
    shares: 8,
    isLiked: false,
    type: 'announcement'
  }
];

export function MainFeed() {
  const [posts, setPosts] = useState<Post[]>(mockPosts);
  const [newPost, setNewPost] = useState('');

  const handleLike = (postId: string) => {
    setPosts(posts.map(post => 
      post.id === postId 
        ? { ...post, isLiked: !post.isLiked, likes: post.isLiked ? post.likes - 1 : post.likes + 1 }
        : post
    ));
  };

  const handleCreatePost = () => {
    if (newPost.trim()) {
      const post: Post = {
        id: Date.now().toString(),
        author: {
          name: 'Ali Uzun',
          department: 'Bilgisayar Mühendisliği',
          university: 'İTÜ',
          avatar: 'AU'
        },
        content: newPost,
        timestamp: 'şimdi',
        likes: 0,
        comments: 0,
        shares: 0,
        isLiked: false,
        type: 'discussion'
      };
      setPosts([post, ...posts]);
      setNewPost('');
    }
  };

  const getTypeColor = (type: string) => {
    switch (type) {
      case 'announcement': return 'bg-primary/10 text-primary border border-primary/20';
      case 'event': return 'bg-secondary text-secondary-foreground border border-border';
      case 'academic': return 'bg-accent text-accent-foreground border border-border';
      default: return 'bg-muted text-muted-foreground border border-border';
    }
  };

  const getTypeLabel = (type: string) => {
    switch (type) {
      case 'announcement': return 'Duyuru';
      case 'event': return 'Etkinlik';
      case 'academic': return 'Akademik';
      default: return 'Tartışma';
    }
  };

  return (
    <div className="h-full overflow-y-auto bg-background">
      <div className="max-w-full mx-auto p-4 pb-20 space-y-4">
        {/* Header */}
        <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 mb-4 border-b border-border">
          <div>
            <h2 className="text-primary">Ana Akış</h2>
            <p className="text-muted-foreground text-sm">Kişiselleştirilmiş içerikler</p>
          </div>
        </div>

        {/* Create Post */}
        <Card>
          <CardContent className="p-4">
            <div className="space-y-4">
              <div className="flex items-start gap-3">
                <div className="w-10 h-10 bg-primary rounded-full flex items-center justify-center">
                  <span className="text-primary-foreground">AU</span>
                </div>
                <div className="flex-1">
                  <Textarea
                    placeholder="Ne düşünüyorsun? Bir araştırma, proje veya etkinlik paylaş..."
                    value={newPost}
                    onChange={(e) => setNewPost(e.target.value)}
                    className="min-h-20 resize-none"
                  />
                </div>
              </div>
              <div className="flex justify-end">
                <Button onClick={handleCreatePost} disabled={!newPost.trim()}>
                  <Plus className="w-4 h-4 mr-2" />
                  Paylaş
                </Button>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Posts */}
        <div className="space-y-4">
          {posts.map((post) => (
            <Card key={post.id}>
              <CardHeader className="pb-3">
                <div className="flex items-start justify-between">
                  <div className="flex items-start gap-3">
                    <div className="w-12 h-12 bg-secondary rounded-full flex items-center justify-center">
                      <span className="text-secondary-foreground">{post.author.avatar}</span>
                    </div>
                    <div>
                      <div className="font-medium">{post.author.name}</div>
                      <div className="text-sm text-muted-foreground">
                        {post.author.department}, {post.author.university}
                      </div>
                      <div className="text-sm text-muted-foreground">{post.timestamp}</div>
                    </div>
                  </div>
                  <div className="flex items-center gap-2">
                    <span className={`px-2 py-1 rounded-full text-xs ${getTypeColor(post.type)}`}>
                      {getTypeLabel(post.type)}
                    </span>
                    <Button variant="ghost" size="sm">
                      <MoreHorizontal className="w-4 h-4" />
                    </Button>
                  </div>
                </div>
              </CardHeader>
              <CardContent className="pt-0">
                <p className="mb-4">{post.content}</p>
                
                <div className="flex items-center justify-between border-t border-border pt-3">
                  <div className="flex items-center gap-6">
                    <Button
                      variant="ghost"
                      size="sm"
                      onClick={() => handleLike(post.id)}
                      className={post.isLiked ? 'text-primary' : ''}
                    >
                      <Heart className={`w-4 h-4 mr-2 ${post.isLiked ? 'fill-current' : ''}`} />
                      {post.likes}
                    </Button>
                    <Button variant="ghost" size="sm">
                      <MessageCircle className="w-4 h-4 mr-2" />
                      {post.comments}
                    </Button>
                    <Button variant="ghost" size="sm">
                      <Share className="w-4 h-4 mr-2" />
                      {post.shares}
                    </Button>
                  </div>
                </div>
              </CardContent>
            </Card>
          ))}
        </div>
      </div>
    </div>
  );
}
