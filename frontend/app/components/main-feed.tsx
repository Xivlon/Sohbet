"use client";

import { useState, useEffect } from 'react';
import { Heart, MessageCircle, Share, MoreHorizontal, Plus } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Textarea } from './ui/textarea';
import { apiClient } from '@/app/lib/api-client';
import { useAuth } from '@/app/contexts/auth-context';
import { toast } from 'sonner';
import { useTranslations } from 'next-intl';

interface Post {
  id: number;
  user_id: number;
  content: string;
  visibility: string;
  created_at: string;
  updated_at?: string;
  user?: {
    id: number;
    username: string;
    name?: string;
    department?: string;
    university?: string;
    avatar_url?: string;
  };
  reaction_count?: number;
  comment_count?: number;
  has_reacted?: boolean;
}

export function MainFeed() {
  const [posts, setPosts] = useState<Post[]>([]);
  const [newPost, setNewPost] = useState('');
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [submitting, setSubmitting] = useState(false);
  const { user } = useAuth();
  const t = useTranslations('feed');
  const tCommon = useTranslations('common');
  const tPost = useTranslations('post');
  const tErrors = useTranslations('errors');

  useEffect(() => {
    // Only load posts if user is authenticated
    if (user) {
      loadPosts();
    }
  }, [user]);

  const loadPosts = async () => {
    setLoading(true);
    setError(null);
    try {
      const response = await apiClient.getPosts(50, 0);
      if (response.data) {
        setPosts(response.data.posts || []);
      } else {
        setError(response.error || t('postError'));
      }
    } catch (err) {
      setError(tErrors('somethingWentWrong'));
      console.error('Failed to load posts:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleLike = async (post: Post) => {
    // Optimistic update
    const wasLiked = post.has_reacted;
    const oldCount = post.reaction_count || 0;

    setPosts(posts.map(p =>
      p.id === post.id
        ? { ...p, has_reacted: !wasLiked, reaction_count: wasLiked ? oldCount - 1 : oldCount + 1 }
        : p
    ));

    try {
      if (wasLiked) {
        await apiClient.removeReaction(post.id);
      } else {
        await apiClient.reactToPost(post.id, 'like');
      }
    } catch (error) {
      // Rollback on error
      setPosts(posts.map(p =>
        p.id === post.id
          ? { ...p, has_reacted: wasLiked, reaction_count: oldCount }
          : p
      ));
      toast.error(t('likeError'));
      console.error('Failed to react to post:', error);
    }
  };

  const handleCreatePost = async () => {
    if (!newPost.trim()) return;

    setSubmitting(true);
    try {
      const response = await apiClient.createPost(newPost.trim(), 'public');
      if (response.data) {
        // Reload posts to get the new post with all metadata
        await loadPosts();
        setNewPost('');
        toast.success(t('postSuccess'));
      } else {
        toast.error(t('deletePostError') + ': ' + (response.error || tCommon('error')));
      }
    } catch (err) {
      toast.error(t('deletePostError'));
      console.error('Failed to create post:', err);
    } finally {
      setSubmitting(false);
    }
  };

  const formatTimestamp = (dateString: string) => {
    const date = new Date(dateString);
    const now = new Date();
    const diffMs = now.getTime() - date.getTime();
    const diffMins = Math.floor(diffMs / 60000);
    const diffHours = Math.floor(diffMins / 60);
    const diffDays = Math.floor(diffHours / 24);

    if (diffMins < 1) return tPost('justNow');
    if (diffMins < 60) return `${diffMins} ${tPost('minutes')} ${tPost('ago')}`;
    if (diffHours < 24) return `${diffHours} ${tPost('hours')} ${tPost('ago')}`;
    if (diffDays < 7) return `${diffDays} ${tPost('days')} ${tPost('ago')}`;

    return date.toLocaleDateString('tr-TR');
  };

  const getInitials = (name?: string, username?: string) => {
    if (name) {
      const parts = name.split(' ');
      return parts.map(p => p[0]).join('').toUpperCase().slice(0, 2);
    }
    return username?.slice(0, 2).toUpperCase() || 'U';
  };

  return (
    <div className="h-full overflow-y-auto bg-background">
      <div className="max-w-3xl mx-auto p-4 md:p-6 lg:p-8 pb-20 md:pb-8 space-y-4">
        {/* Header */}
        <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 md:-mx-6 lg:-mx-8 mb-4 border-b border-border">
          <div className="max-w-3xl mx-auto px-4 md:px-6 lg:px-8">
            <h2 className="text-primary text-xl md:text-2xl font-semibold">Ana Akış</h2>
            <p className="text-muted-foreground text-sm md:text-base">Kişiselleştirilmiş içerikler</p>
          </div>
        </div>

        {/* Create Post */}
        {user && (
          <Card>
            <CardContent className="p-4">
              <div className="space-y-4">
                <div className="flex items-start gap-3">
                  <div className="w-10 h-10 bg-primary rounded-full flex items-center justify-center flex-shrink-0">
                    <span className="text-primary-foreground text-sm font-medium">
                      {getInitials(user.name, user.username)}
                    </span>
                  </div>
                  <div className="flex-1">
                    <Textarea
                      placeholder={t('whatsOnYourMind')}
                      value={newPost}
                      onChange={(e) => setNewPost(e.target.value)}
                      className="min-h-20 resize-none"
                      disabled={submitting}
                    />
                  </div>
                </div>
                <div className="flex justify-end">
                  <Button onClick={handleCreatePost} disabled={!newPost.trim() || submitting}>
                    <Plus className="w-4 h-4 mr-2" />
                    {submitting ? tCommon('processing') + '...' : tPost('share')}
                  </Button>
                </div>
              </div>
            </CardContent>
          </Card>
        )}

        {/* Posts */}
        {loading ? (
          <div className="text-center py-8">
            <p className="text-muted-foreground">{tCommon('loading')}</p>
          </div>
        ) : error ? (
          <Card>
            <CardContent className="p-8 text-center">
              <p className="text-destructive mb-4">{error}</p>
              <Button onClick={loadPosts}>{tErrors('tryAgain')}</Button>
            </CardContent>
          </Card>
        ) : posts.length === 0 ? (
          <Card>
            <CardContent className="p-8 text-center">
              <p className="text-muted-foreground">{t('noPostsYet')}</p>
            </CardContent>
          </Card>
        ) : (
          <div className="space-y-4">
            {posts.map((post) => (
              <Card key={post.id}>
                <CardHeader className="pb-3">
                  <div className="flex items-start justify-between">
                    <div className="flex items-start gap-3">
                      <div className="w-12 h-12 bg-secondary rounded-full flex items-center justify-center flex-shrink-0">
                        <span className="text-secondary-foreground text-sm font-medium">
                          {getInitials(post.user?.name, post.user?.username)}
                        </span>
                      </div>
                      <div>
                        <div className="font-medium">{post.user?.name || post.user?.username || 'Anonim'}</div>
                        {(post.user?.department || post.user?.university) && (
                          <div className="text-sm text-muted-foreground">
                            {post.user?.department && post.user?.university 
                              ? `${post.user.department}, ${post.user.university}`
                              : post.user?.department || post.user?.university}
                          </div>
                        )}
                        <div className="text-sm text-muted-foreground">{formatTimestamp(post.created_at)}</div>
                      </div>
                    </div>
                    <Button variant="ghost" size="sm">
                      <MoreHorizontal className="w-4 h-4" />
                    </Button>
                  </div>
                </CardHeader>
                <CardContent className="pt-0">
                  <p className="mb-4 whitespace-pre-wrap">{post.content}</p>
                  
                  <div className="flex items-center justify-between border-t border-border pt-3">
                    <div className="flex items-center gap-6">
                      <Button
                        variant="ghost"
                        size="sm"
                        onClick={() => handleLike(post)}
                        className={post.has_reacted ? 'text-primary' : ''}
                      >
                        <Heart className={`w-4 h-4 mr-2 ${post.has_reacted ? 'fill-current' : ''}`} />
                        {post.reaction_count || 0}
                      </Button>
                      <Button variant="ghost" size="sm">
                        <MessageCircle className="w-4 h-4 mr-2" />
                        {post.comment_count || 0}
                      </Button>
                      <Button variant="ghost" size="sm">
                        <Share className="w-4 h-4 mr-2" />
                        0
                      </Button>
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
