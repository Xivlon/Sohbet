import { NextIntlClientProvider } from 'next-intl';
import { getMessages } from 'next-intl/server';
import { notFound } from 'next/navigation';
import { locales } from '../../i18n';

export default async function LocaleLayout({
  children,
  params
}: {
  children: React.ReactNode;
  params: Promise<{ locale: string }>;
}) {
  // Await params in Next.js 16
  const { locale } = await params;

  // Validate locale
  if (!locales.includes(locale as any)) {
    notFound();
  }

  // Get messages for the locale
  const messages = await getMessages();

  return (
    <NextIntlClientProvider messages={messages}>
      {children}
    </NextIntlClientProvider>
  );
}
