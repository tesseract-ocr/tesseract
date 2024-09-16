import type { MiddlewareMatcher } from '../../analysis/get-page-static-info';
export type MiddlewareLoaderOptions = {
    absolutePagePath: string;
    page: string;
    rootDir: string;
    matchers?: string;
    preferredRegion: string | string[] | undefined;
    middlewareConfig: string;
};
export declare function encodeMatchers(matchers: MiddlewareMatcher[]): string;
export declare function decodeMatchers(encodedMatchers: string): MiddlewareMatcher[];
export default function middlewareLoader(this: any): Promise<string>;
