'use client';

import React, { useState, useRef } from 'react';
import { apiClient, Media } from '../lib/api-client';

interface FileUploadProps {
  userId: number;
  mediaType: string;
  accept?: string;
  maxSizeMB?: number;
  onUploadSuccess?: (media: Media) => void;
  onUploadError?: (error: string) => void;
  children?: React.ReactNode;
  className?: string;
}

export function FileUpload({
  userId,
  mediaType,
  accept = 'image/jpeg,image/png,image/gif,image/webp',
  maxSizeMB = 5,
  onUploadSuccess,
  onUploadError,
  children,
  className = '',
}: FileUploadProps) {
  const [uploading, setUploading] = useState(false);
  const [progress, setProgress] = useState(0);
  const [error, setError] = useState<string | null>(null);
  const fileInputRef = useRef<HTMLInputElement>(null);

  const handleFileSelect = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    // Validate file size
    const maxSizeBytes = maxSizeMB * 1024 * 1024;
    if (file.size > maxSizeBytes) {
      const errorMsg = `File size exceeds ${maxSizeMB}MB limit`;
      setError(errorMsg);
      if (onUploadError) onUploadError(errorMsg);
      return;
    }

    // Validate file type
    if (accept && !accept.split(',').some(type => file.type.match(type.trim()))) {
      const errorMsg = 'Invalid file type';
      setError(errorMsg);
      if (onUploadError) onUploadError(errorMsg);
      return;
    }

    setUploading(true);
    setError(null);
    setProgress(0);

    try {
      // Simulate progress for better UX
      const progressInterval = setInterval(() => {
        setProgress(prev => Math.min(prev + 10, 90));
      }, 100);

      const response = await apiClient.uploadMedia(file, userId, mediaType);

      clearInterval(progressInterval);
      setProgress(100);

      if (response.error || !response.data) {
        throw new Error(response.error || 'Upload failed');
      }

      if (onUploadSuccess) {
        onUploadSuccess(response.data);
      }

      // Reset after success
      setTimeout(() => {
        setProgress(0);
        setUploading(false);
      }, 500);
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : 'Upload failed';
      setError(errorMsg);
      if (onUploadError) onUploadError(errorMsg);
      setUploading(false);
      setProgress(0);
    }

    // Reset file input
    if (fileInputRef.current) {
      fileInputRef.current.value = '';
    }
  };

  const handleClick = () => {
    fileInputRef.current?.click();
  };

  return (
    <div className={className}>
      <input
        ref={fileInputRef}
        type="file"
        accept={accept}
        onChange={handleFileSelect}
        style={{ display: 'none' }}
        disabled={uploading}
      />
      
      <div onClick={handleClick} style={{ cursor: uploading ? 'wait' : 'pointer' }}>
        {children || (
          <button
            type="button"
            disabled={uploading}
            className="px-4 py-2 bg-blue-500 text-white rounded hover:bg-blue-600 disabled:bg-gray-400 disabled:cursor-not-allowed"
          >
            {uploading ? `Uploading... ${progress}%` : 'Upload File'}
          </button>
        )}
      </div>

      {uploading && (
        <div className="mt-2 w-full bg-gray-200 rounded-full h-2">
          <div
            className="bg-blue-500 h-2 rounded-full transition-all duration-300"
            style={{ width: `${progress}%` }}
          />
        </div>
      )}

      {error && (
        <div className="mt-2 text-red-500 text-sm">
          {error}
        </div>
      )}
    </div>
  );
}
