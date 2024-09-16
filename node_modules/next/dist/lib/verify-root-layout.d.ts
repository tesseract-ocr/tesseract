import type { PageExtensions } from '../build/page-extensions-type';
export declare function verifyRootLayout({ dir, appDir, tsconfigPath, pagePath, pageExtensions, }: {
    dir: string;
    appDir: string;
    tsconfigPath: string;
    pagePath: string;
    pageExtensions: PageExtensions;
}): Promise<[boolean, string | undefined]>;
