import { getRequestConfig } from 'next-intl/server';

// List of all locales that are supported
export const locales = ['en', 'tr'] as const;

// The locale prefixing strategy
export const localePrefix = 'always' as const;

export default getRequestConfig(async ({ requestLocale }) => {
  // This typically corresponds to the `[locale]` segment
  let locale = await requestLocale;

  // Ensure that a valid locale is used
  if (!locale || !locales.includes(locale as (typeof locales)[number])) {
    locale = 'en';
  }

  return {
    locale,
    messages: (await import(`./messages/${locale}.json`)).default
  };
});
