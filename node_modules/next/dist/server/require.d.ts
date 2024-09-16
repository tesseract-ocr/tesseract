import type { FontManifest } from './font-utils';
export declare function getMaybePagePath(page: string, distDir: string, locales: string[] | undefined, isAppPath: boolean): string | null;
export declare function getPagePath(page: string, distDir: string, locales: string[] | undefined, isAppPath: boolean): string;
export declare function requirePage(page: string, distDir: string, isAppPath: boolean): any;
export declare function requireFontManifest(distDir: string): FontManifest;
