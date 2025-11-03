-- Migration: Seed Turkish Organizations
-- Date: November 3, 2025
-- Description: Adds mock Turkish university organizations, clubs, and societies

-- =============================================================================
-- TURKISH ORGANIZATIONS SEED DATA
-- =============================================================================

-- Technology & Engineering Clubs
INSERT OR IGNORE INTO organizations (name, type, description, email, website) VALUES
('Yazılım Geliştirme Kulübü', 'technology', 'Yazılım geliştirme, web programlama ve mobil uygulama geliştirme konularında etkinlikler düzenleyen teknoloji kulübü', 'yazilim@universite.edu.tr', 'https://yazilimkulubu.org'),
('Robotik ve Otomasyon Topluluğu', 'technology', 'Robot tasarımı, otomasyon sistemleri ve yapay zeka üzerine çalışan öğrenci topluluğu', 'robotik@universite.edu.tr', 'https://robotik-toplulugu.org'),
('Siber Güvenlik Kulübü', 'technology', 'Siber güvenlik, etik hacking ve bilgi güvenliği konularında eğitimler veren topluluk', 'siberkul@universite.edu.tr', 'https://sibergüvenlik.org'),
('IEEE Öğrenci Kolu', 'technology', 'Elektrik-elektronik mühendisliği ve bilgisayar bilimleri alanında uluslararası öğrenci organizasyonu', 'ieee@universite.edu.tr', 'https://ieee-turkiye.org'),
('Açık Kaynak Yazılım Topluluğu', 'technology', 'Açık kaynak projelere katkı sağlayan ve Linux sistemleri üzerine çalışan teknoloji topluluğu', 'opensource@universite.edu.tr', 'https://acikkaynak.org'),

-- Science & Academic Clubs
('Bilim ve Araştırma Kulübü', 'academic', 'Bilimsel araştırmalar, deneyler ve projeler geliştiren akademik topluluk', 'bilim@universite.edu.tr', 'https://bilimkulubu.org'),
('Matematik ve İstatistik Derneği', 'academic', 'Matematik ve istatistik alanında seminerler, yarışmalar düzenleyen öğrenci derneği', 'matematik@universite.edu.tr', 'https://matkulup.org'),
('Fizik Araştırmaları Topluluğu', 'academic', 'Teorik ve deneysel fizik çalışmaları yapan akademik topluluk', 'fizik@universite.edu.tr', 'https://fiziktopluluk.org'),
('Kimya Mühendisliği Kulübü', 'academic', 'Kimya mühendisliği öğrencilerinin proje ve araştırmalar yaptığı kulüp', 'kimya@universite.edu.tr', 'https://kimyamuh.org'),
('Astronomi Gözlem Topluluğu', 'academic', 'Gök gözlemleri, astronomi etkinlikleri ve uzay bilimleri üzerine çalışan topluluk', 'astronomi@universite.edu.tr', 'https://astronomitr.org'),

-- Sports Clubs
('Üniversite Futbol Takımı', 'sports', 'Üniversite futbol takımı ve spor etkinlikleri organizasyonu', 'futbol@universite.edu.tr', 'https://futbol-takimi.org'),
('Basketbol Kulübü', 'sports', 'Basketbol antrenmanları ve turnuvaları düzenleyen spor kulübü', 'basketbol@universite.edu.tr', 'https://basket-kulup.org'),
('Voleybol Topluluğu', 'sports', 'Voleybol maçları ve spor etkinlikleri organize eden öğrenci topluluğu', 'voleybol@universite.edu.tr', 'https://voleybol.org'),
('Dağcılık ve Doğa Sporları Kulübü', 'sports', 'Dağcılık, kamp, trekking ve doğa sporları etkinlikleri düzenleyen kulüp', 'dagcilik@universite.edu.tr', 'https://dogasporlari.org'),
('Masa Tenisi Kulübü', 'sports', 'Masa tenisi antrenmanları ve turnuvaları organize eden spor kulübü', 'masatenisi@universite.edu.tr', 'https://masatenisi.org'),
('Satranç Topluluğu', 'sports', 'Satranç turnuvaları ve eğitimleri düzenleyen stratejik oyun kulübü', 'satranc@universite.edu.tr', 'https://satranckulup.org'),

-- Arts & Culture Clubs
('Tiyatro Kulübü', 'arts', 'Oyun gösterimleri, tiyatro atölyeleri ve sahneleme çalışmaları yapan sanat kulübü', 'tiyatro@universite.edu.tr', 'https://tiyatrokulup.org'),
('Müzik Topluluğu', 'arts', 'Konserler, müzik dinletileri ve enstrüman eğitimleri veren müzik topluluğu', 'muzik@universite.edu.tr', 'https://muziktopluluk.org'),
('Fotoğrafçılık Kulübü', 'arts', 'Fotoğraf sergisi, atölye çalışmaları ve fotoğraf gezileri düzenleyen sanat kulübü', 'fotograf@universite.edu.tr', 'https://fotografkulup.org'),
('Sinema ve Medya Kulübü', 'arts', 'Film gösterimleri, kısa film yarışmaları ve medya projeleri geliştiren kulüp', 'sinema@universite.edu.tr', 'https://sinemakulup.org'),
('Edebiyat ve Yazarlık Topluluğu', 'arts', 'Edebiyat söyleşileri, şiir dinletileri ve yazarlık atölyeleri düzenleyen topluluk', 'edebiyat@universite.edu.tr', 'https://edebiyat.org'),
('Halk Dansları Topluluğu', 'arts', 'Türk halk dansları gösterileri ve dans eğitimleri veren kültür topluluğu', 'halkdanslari@universite.edu.tr', 'https://halkdanslari.org'),

-- Social & Volunteer Organizations
('Sosyal Sorumluluk Kulübü', 'social', 'Toplum hizmeti, gönüllülük projeleri ve yardım kampanyaları düzenleyen sosyal kulüp', 'sosyal@universite.edu.tr', 'https://sosyalsorumluluk.org'),
('Çevre ve Doğa Koruma Topluluğu', 'social', 'Çevre bilinci, geri dönüşüm ve doğa koruma projeleri yürüten topluluk', 'cevre@universite.edu.tr', 'https://cevrekoruma.org'),
('Kızılay Gönüllüleri', 'social', 'Kan bağışı kampanyaları ve insani yardım faaliyetleri organize eden gönüllü grubu', 'kizilay@universite.edu.tr', 'https://kizilaygonulluleri.org'),
('Eğitim Gönüllüleri Topluluğu', 'social', 'Eğitim desteği, ders verme ve mentorluk programları düzenleyen sosyal topluluk', 'egitim@universite.edu.tr', 'https://egitimgonulluleri.org'),
('Hayvan Hakları ve Koruma Derneği', 'social', 'Sokak hayvanları için yardım, barınak desteği ve farkındalık çalışmaları yapan dernek', 'hayvansever@universite.edu.tr', 'https://hayvanhaklari.org'),

-- Business & Entrepreneurship
('Girişimcilik Kulübü', 'technology', 'Startup ekosistemi, iş geliştirme ve girişimcilik eğitimleri veren kulüp', 'girisimcilik@universite.edu.tr', 'https://girisimcilik.org'),
('Finans ve Yatırım Topluluğu', 'academic', 'Finansal okur-yazarlık, yatırım seminerleri ve borsa simülasyonları düzenleyen topluluk', 'finans@universite.edu.tr', 'https://finans-topluluk.org'),
('Pazarlama ve İletişim Kulübü', 'academic', 'Dijital pazarlama, marka yönetimi ve iletişim stratejileri üzerine çalışan kulüp', 'pazarlama@universite.edu.tr', 'https://pazarlama.org'),

-- International & Language Clubs
('Uluslararası Öğrenci Topluluğu', 'social', 'Kültürlerarası etkinlikler, dil değişimi ve uluslararası öğrenci entegrasyonu sağlayan topluluk', 'uluslararasi@universite.edu.tr', 'https://international-students.org'),
('İngilizce Konuşma Kulübü', 'academic', 'İngilizce pratik yapma, konuşma kulübü ve dil gelişim etkinlikleri düzenleyen kulüp', 'english@universite.edu.tr', 'https://englishclub.org'),
('Almanca Dil ve Kültür Topluluğu', 'academic', 'Almanca dil eğitimi ve Alman kültürü tanıtım etkinlikleri düzenleyen topluluk', 'deutsch@universite.edu.tr', 'https://deutschklub.org'),

-- Gaming & Entertainment
('E-Spor ve Oyun Topluluğu', 'technology', 'E-spor turnuvaları, oyun geliştirme ve dijital oyun kültürü üzerine çalışan topluluk', 'esports@universite.edu.tr', 'https://esports-topluluk.org'),
('Masa Oyunları Kulübü', 'social', 'Strateji oyunları, rol yapma oyunları ve masa oyunu turnuvaları düzenleyen kulüp', 'masaoyun@universite.edu.tr', 'https://masaoyunlari.org'),

-- Debate & Public Speaking
('Münazara ve Tartışma Kulübü', 'academic', 'Münazara turnuvaları, kamu konuşması ve tartışma teknikleri eğitimi veren kulüp', 'munazara@universite.edu.tr', 'https://munazara.org'),
('Model Birleşmiş Milletler Topluluğu', 'academic', 'Uluslararası ilişkiler simülasyonu ve diplomasi eğitimi sağlayan akademik topluluk', 'mun@universite.edu.tr', 'https://mun-turkiye.org');

-- =============================================================================
-- SEED DATA SUMMARY
-- =============================================================================
-- Total organizations added: 40
-- Categories:
--   - Technology & Engineering: 5
--   - Science & Academic: 5
--   - Sports: 6
--   - Arts & Culture: 6
--   - Social & Volunteer: 5
--   - Business & Entrepreneurship: 3
--   - International & Language: 3
--   - Gaming & Entertainment: 2
--   - Debate & Public Speaking: 2
-- =============================================================================
