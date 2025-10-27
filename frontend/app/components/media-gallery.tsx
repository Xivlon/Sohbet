'use client';

import React, { useState, useEffect } from 'react';
import { apiClient, Media } from '../lib/api-client';

interface MediaGalleryProps {
  userId: number;
  mediaType?: string;
  columns?: number;
}

export function MediaGallery({ userId, mediaType, columns = 3 }: MediaGalleryProps) {
  const [media, setMedia] = useState<Media[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    loadMedia();
  }, [userId, mediaType]);

  const loadMedia = async () => {
    setLoading(true);
    setError(null);

    try {
      const response = await apiClient.getUserMedia(userId);

      if (response.error || !response.data) {
        throw new Error(response.error || 'Failed to load media');
      }

      let mediaList = response.data;
      
      // Filter by media type if specified
      if (mediaType) {
        mediaList = mediaList.filter(m => m.media_type === mediaType);
      }

      setMedia(mediaList);
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : 'Failed to load media';
      setError(errorMsg);
    } finally {
      setLoading(false);
    }
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center p-8">
        <div className="text-gray-500">Loading media...</div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex items-center justify-center p-8">
        <div className="text-red-500">Error: {error}</div>
      </div>
    );
  }

  if (media.length === 0) {
    return (
      <div className="flex items-center justify-center p-8">
        <div className="text-gray-500">No media uploaded yet</div>
      </div>
    );
  }

  const gridClass = `grid gap-4 grid-cols-${columns}`;

  return (
    <div className={gridClass}>
      {media.map((item) => (
        <MediaItem key={item.id} media={item} />
      ))}
    </div>
  );
}

function MediaItem({ media }: { media: Media }) {
  const imageUrl = media.url || apiClient.getMediaUrl(media.storage_key);
  const isImage = media.mime_type?.startsWith('image/');

  return (
    <div className="relative aspect-square bg-gray-100 rounded-lg overflow-hidden border border-gray-200 hover:shadow-lg transition-shadow">
      {isImage ? (
        <img
          src={imageUrl}
          alt={media.file_name || 'Uploaded media'}
          className="w-full h-full object-cover"
        />
      ) : (
        <div className="w-full h-full flex items-center justify-center">
          <div className="text-center p-4">
            <div className="text-gray-400 mb-2">📄</div>
            <div className="text-xs text-gray-600 truncate">
              {media.file_name || 'File'}
            </div>
          </div>
        </div>
      )}
      
      {media.file_name && (
        <div className="absolute bottom-0 left-0 right-0 bg-black bg-opacity-50 text-white text-xs p-2 truncate">
          {media.file_name}
        </div>
      )}
    </div>
  );
}
