'use client';

import React, { useState } from 'react';
import { apiClient, Media, User } from '../lib/api-client';
import { FileUpload } from './file-upload';

interface AvatarUploadProps {
  user: User;
  onAvatarUpdate?: (avatarUrl: string) => void;
  size?: 'sm' | 'md' | 'lg';
}

export function AvatarUpload({ user, onAvatarUpdate, size = 'md' }: AvatarUploadProps) {
  const [avatarUrl, setAvatarUrl] = useState<string | null>(
    user.avatar_url || null
  );
  const [previewUrl, setPreviewUrl] = useState<string | null>(null);

  const sizeClasses = {
    sm: 'w-16 h-16',
    md: 'w-24 h-24',
    lg: 'w-32 h-32',
  };

  const handleUploadSuccess = (media: Media) => {
    if (media.url) {
      const fullUrl = apiClient.getMediaUrl(media.storage_key);
      setAvatarUrl(fullUrl);
      setPreviewUrl(null);
      if (onAvatarUpdate) {
        onAvatarUpdate(fullUrl);
      }
    }
  };

  const handleUploadError = (error: string) => {
    console.error('Avatar upload error:', error);
    setPreviewUrl(null);
  };

  const getDisplayUrl = () => {
    if (previewUrl) return previewUrl;
    if (avatarUrl) return avatarUrl;
    return null;
  };

  const displayUrl = getDisplayUrl();
  const initials = user.username.substring(0, 2).toUpperCase();

  return (
    <div className="flex flex-col items-center gap-3">
      <div className={`${sizeClasses[size]} relative rounded-full overflow-hidden border-2 border-gray-300 bg-gray-100`}>
        {displayUrl ? (
          <img
            src={displayUrl}
            alt={`${user.username}'s avatar`}
            className="w-full h-full object-cover"
          />
        ) : (
          <div className="w-full h-full flex items-center justify-center bg-gradient-to-br from-blue-400 to-purple-500 text-white font-bold text-lg">
            {initials}
          </div>
        )}
      </div>

      <FileUpload
        userId={user.id}
        mediaType="avatar"
        accept="image/jpeg,image/png,image/webp"
        maxSizeMB={2}
        onUploadSuccess={handleUploadSuccess}
        onUploadError={handleUploadError}
      >
        <button
          type="button"
          className="px-4 py-2 text-sm bg-white border border-gray-300 rounded-md hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-colors"
        >
          Change Avatar
        </button>
      </FileUpload>
    </div>
  );
}
