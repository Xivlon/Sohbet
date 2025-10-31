"use client";
import { useState, useEffect } from 'react';
import { useAuth } from '../contexts/auth-context';
import { apiClient } from '../lib/api-client';
import { API_URL } from '../lib/config';
import { User as UserIcon, Mail, Building2, BookOpen, GraduationCap, Phone, MapPin, Edit2, Save, X } from 'lucide-react';

export function Profile() {
  const { user, isLoading } = useAuth();
  const [isEditing, setIsEditing] = useState(false);
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');
  
  const [formData, setFormData] = useState({
    name: '',
    position: '',
    phone_number: '',
    university: '',
    department: '',
    enrollment_year: new Date().getFullYear(),
    primary_language: 'Turkish',
  });

  useEffect(() => {
    if (user) {
      setFormData({
        name: user.name || '',
        position: user.position || '',
        phone_number: user.phone_number || '',
        university: user.university || '',
        department: user.department || '',
        enrollment_year: user.enrollment_year || new Date().getFullYear(),
        primary_language: user.primary_language || 'Turkish',
      });
    }
  }, [user]);

  const handleInputChange = (field: string, value: string | number) => {
    setFormData(prev => ({ ...prev, [field]: value }));
    setError('');
    setSuccess('');
  };

  const handleSave = async () => {
    if (!user?.id) return;

    setSaving(true);
    setError('');
    setSuccess('');

    try {
      const response = await fetch(`${API_URL || 'http://localhost:8080'}/api/users/${user.id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${apiClient.getToken()}`,
        },
        body: JSON.stringify(formData),
      });

      const data = await response.json();

      if (!response.ok) {
        setError(data.error || 'Profil güncellenemedi');
        return;
      }

      apiClient.setUser(data);
      setSuccess('Profil başarıyla güncellendi!');
      setIsEditing(false);
    } catch {
      setError('Bir hata oluştu');
    } finally {
      setSaving(false);
    }
  };

  const handleCancel = () => {
    setFormData({
      name: user?.name || '',
      position: user?.position || '',
      phone_number: user?.phone_number || '',
      university: user?.university || '',
      department: user?.department || '',
      enrollment_year: user?.enrollment_year || new Date().getFullYear(),
      primary_language: user?.primary_language || 'Turkish',
    });
    setIsEditing(false);
    setError('');
    setSuccess('');
  };

  if (isLoading) {
    return (
      <div className="flex items-center justify-center h-full">
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-red-600"></div>
      </div>
    );
  }

  if (!user) {
    return (
      <div className="flex items-center justify-center h-full">
        <p className="text-gray-500">Kullanıcı bulunamadı</p>
      </div>
    );
  }

  return (
    <div className="h-full overflow-y-auto">
      <div className="max-w-4xl mx-auto p-6">
        <div className="bg-white dark:bg-gray-800 rounded-lg shadow-md overflow-hidden">
          <div className="bg-gradient-to-r from-red-600 to-red-700 dark:from-red-700 dark:to-red-800 h-32"></div>
          
          <div className="relative px-6 pb-6">
            <div className="flex items-end justify-between -mt-16 mb-6">
              <div className="flex items-end gap-4">
                <div className="w-32 h-32 bg-white dark:bg-gray-700 rounded-full border-4 border-white dark:border-gray-800 flex items-center justify-center">
                  <UserIcon className="w-16 h-16 text-red-600 dark:text-red-500" />
                </div>
                <div className="pb-2">
                  <h1 className="text-2xl font-bold text-gray-900 dark:text-white">
                    {user.name || user.username}
                  </h1>
                  <p className="text-gray-600 dark:text-gray-400">@{user.username}</p>
                </div>
              </div>
              
              {!isEditing && (
                <button
                  onClick={() => setIsEditing(true)}
                  className="flex items-center gap-2 px-4 py-2 bg-red-600 hover:bg-red-700 dark:bg-red-700 dark:hover:bg-red-600 text-white rounded-md transition"
                >
                  <Edit2 className="w-4 h-4" />
                  Profili Düzenle
                </button>
              )}
            </div>

            {error && (
              <div className="mb-4 p-3 bg-red-100 dark:bg-red-900/30 text-red-700 dark:text-red-300 rounded-md text-sm">
                {error}
              </div>
            )}

            {success && (
              <div className="mb-4 p-3 bg-green-100 dark:bg-green-900/30 text-green-700 dark:text-green-300 rounded-md text-sm">
                {success}
              </div>
            )}

            <div className="grid md:grid-cols-2 gap-6">
              <div>
                <label className="block text-sm font-medium mb-2 text-gray-700 dark:text-gray-300">
                  <Mail className="w-4 h-4 inline mr-2" />
                  E-posta
                </label>
                <div className="px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md bg-gray-50 dark:bg-gray-700/50 text-gray-900 dark:text-white">
                  {user.email}
                </div>
              </div>

              <div>
                <label className="block text-sm font-medium mb-2 text-gray-700 dark:text-gray-300">
                  <UserIcon className="w-4 h-4 inline mr-2" />
                  Ad Soyad
                </label>
                {isEditing ? (
                  <input
                    type="text"
                    value={formData.name}
                    onChange={(e) => handleInputChange('name', e.target.value)}
                    className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                    placeholder="Ad Soyad"
                  />
                ) : (
                  <div className="px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md bg-gray-50 dark:bg-gray-700/50 text-gray-900 dark:text-white">
                    {user.name || '-'}
                  </div>
                )}
              </div>

              <div>
                <label className="block text-sm font-medium mb-2 text-gray-700 dark:text-gray-300">
                  <MapPin className="w-4 h-4 inline mr-2" />
                  Pozisyon
                </label>
                {isEditing ? (
                  <select
                    value={formData.position}
                    onChange={(e) => handleInputChange('position', e.target.value)}
                    className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                  >
                    <option value="">Seçiniz</option>
                    <option value="Professor">Profesör</option>
                    <option value="Researcher">Araştırmacı</option>
                    <option value="Engineer">Mühendis</option>
                    <option value="Technician">Teknisyen</option>
                    <option value="Student">Öğrenci</option>
                  </select>
                ) : (
                  <div className="px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md bg-gray-50 dark:bg-gray-700/50 text-gray-900 dark:text-white">
                    {user.position || '-'}
                  </div>
                )}
              </div>

              <div>
                <label className="block text-sm font-medium mb-2 text-gray-700 dark:text-gray-300">
                  <Phone className="w-4 h-4 inline mr-2" />
                  Telefon
                </label>
                {isEditing ? (
                  <input
                    type="tel"
                    value={formData.phone_number}
                    onChange={(e) => handleInputChange('phone_number', e.target.value)}
                    className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                    placeholder="+90-XXX-XXX-XXXX"
                  />
                ) : (
                  <div className="px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md bg-gray-50 dark:bg-gray-700/50 text-gray-900 dark:text-white">
                    {user.phone_number || '-'}
                  </div>
                )}
              </div>

              <div>
                <label className="block text-sm font-medium mb-2 text-gray-700 dark:text-gray-300">
                  <Building2 className="w-4 h-4 inline mr-2" />
                  Üniversite
                </label>
                {isEditing ? (
                  <input
                    type="text"
                    value={formData.university}
                    onChange={(e) => handleInputChange('university', e.target.value)}
                    className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                    placeholder="İstanbul Teknik Üniversitesi"
                  />
                ) : (
                  <div className="px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md bg-gray-50 dark:bg-gray-700/50 text-gray-900 dark:text-white">
                    {user.university || '-'}
                  </div>
                )}
              </div>

              <div>
                <label className="block text-sm font-medium mb-2 text-gray-700 dark:text-gray-300">
                  <BookOpen className="w-4 h-4 inline mr-2" />
                  Bölüm
                </label>
                {isEditing ? (
                  <input
                    type="text"
                    value={formData.department}
                    onChange={(e) => handleInputChange('department', e.target.value)}
                    className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                    placeholder="Bilgisayar Mühendisliği"
                  />
                ) : (
                  <div className="px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md bg-gray-50 dark:bg-gray-700/50 text-gray-900 dark:text-white">
                    {user.department || '-'}
                  </div>
                )}
              </div>

              <div>
                <label className="block text-sm font-medium mb-2 text-gray-700 dark:text-gray-300">
                  <GraduationCap className="w-4 h-4 inline mr-2" />
                  Kayıt Yılı
                </label>
                {isEditing ? (
                  <input
                    type="number"
                    value={formData.enrollment_year}
                    onChange={(e) => handleInputChange('enrollment_year', parseInt(e.target.value))}
                    className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                    min="1900"
                    max={new Date().getFullYear() + 10}
                  />
                ) : (
                  <div className="px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md bg-gray-50 dark:bg-gray-700/50 text-gray-900 dark:text-white">
                    {user.enrollment_year || '-'}
                  </div>
                )}
              </div>

              <div>
                <label className="block text-sm font-medium mb-2 text-gray-700 dark:text-gray-300">
                  Ana Dil
                </label>
                {isEditing ? (
                  <input
                    type="text"
                    value={formData.primary_language}
                    onChange={(e) => handleInputChange('primary_language', e.target.value)}
                    className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md focus:outline-none focus:ring-2 focus:ring-red-500 bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
                    placeholder="Türkçe"
                  />
                ) : (
                  <div className="px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-md bg-gray-50 dark:bg-gray-700/50 text-gray-900 dark:text-white">
                    {user.primary_language || '-'}
                  </div>
                )}
              </div>
            </div>

            {isEditing && (
              <div className="flex justify-end gap-3 mt-6">
                <button
                  onClick={handleCancel}
                  disabled={saving}
                  className="flex items-center gap-2 px-4 py-2 border border-gray-300 dark:border-gray-600 rounded-md text-gray-700 dark:text-gray-300 hover:bg-gray-50 dark:hover:bg-gray-700 transition disabled:opacity-50"
                >
                  <X className="w-4 h-4" />
                  İptal
                </button>
                <button
                  onClick={handleSave}
                  disabled={saving}
                  className="flex items-center gap-2 px-4 py-2 bg-red-600 hover:bg-red-700 dark:bg-red-700 dark:hover:bg-red-600 text-white rounded-md transition disabled:opacity-50"
                >
                  <Save className="w-4 h-4" />
                  {saving ? 'Kaydediliyor...' : 'Kaydet'}
                </button>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}
