/// <reference types="node" />
/// <reference types="node" />
/// <reference types="node" />
import type { IncomingMessage, ServerResponse } from 'http';
import { type UrlWithParsedQuery } from 'url';
import type { ImageConfigComplete } from '../shared/lib/image-config';
import type { NextConfigComplete } from './config-shared';
import type { NextUrlWithParsedQuery } from './request-meta';
import type { IncrementalCacheEntry, IncrementalCacheValue } from './response-cache';
type XCacheHeader = 'MISS' | 'HIT' | 'STALE';
export interface ImageParamsResult {
    href: string;
    isAbsolute: boolean;
    isStatic: boolean;
    width: number;
    quality: number;
    mimeType: string;
    sizes: number[];
    minimumCacheTTL: number;
}
interface ImageUpstream {
    buffer: Buffer;
    contentType: string | null | undefined;
    cacheControl: string | null | undefined;
}
export declare function getHash(items: (string | number | Buffer)[]): string;
/**
 * Inspects the first few bytes of a buffer to determine if
 * it matches the "magic number" of known file signatures.
 * https://en.wikipedia.org/wiki/List_of_file_signatures
 */
export declare function detectContentType(buffer: Buffer): "image/svg+xml" | "image/avif" | "image/webp" | "image/jpeg" | "image/png" | "image/x-icon" | "image/gif" | null;
export declare class ImageOptimizerCache {
    private cacheDir;
    private nextConfig;
    static validateParams(req: IncomingMessage, query: UrlWithParsedQuery['query'], nextConfig: NextConfigComplete, isDev: boolean): ImageParamsResult | {
        errorMessage: string;
    };
    static getCacheKey({ href, width, quality, mimeType, }: {
        href: string;
        width: number;
        quality: number;
        mimeType: string;
    }): string;
    constructor({ distDir, nextConfig, }: {
        distDir: string;
        nextConfig: NextConfigComplete;
    });
    get(cacheKey: string): Promise<IncrementalCacheEntry | null>;
    set(cacheKey: string, value: IncrementalCacheValue | null, { revalidate, }: {
        revalidate?: number | false;
    }): Promise<void>;
}
export declare class ImageError extends Error {
    statusCode: number;
    constructor(statusCode: number, message: string);
}
export declare function getMaxAge(str: string | null | undefined): number;
export declare function optimizeImage({ buffer, contentType, quality, width, height, nextConfigOutput, }: {
    buffer: Buffer;
    contentType: string;
    quality: number;
    width: number;
    height?: number;
    nextConfigOutput?: 'standalone' | 'export';
}): Promise<Buffer>;
export declare function fetchExternalImage(href: string): Promise<ImageUpstream>;
export declare function fetchInternalImage(href: string, _req: IncomingMessage, _res: ServerResponse, handleRequest: (newReq: IncomingMessage, newRes: ServerResponse, newParsedUrl?: NextUrlWithParsedQuery) => Promise<void>): Promise<ImageUpstream>;
export declare function imageOptimizer(imageUpstream: ImageUpstream, paramsResult: Pick<ImageParamsResult, 'href' | 'width' | 'quality' | 'mimeType'>, nextConfig: {
    output: NextConfigComplete['output'];
    images: Pick<NextConfigComplete['images'], 'dangerouslyAllowSVG' | 'minimumCacheTTL'>;
}, isDev: boolean | undefined): Promise<{
    buffer: Buffer;
    contentType: string;
    maxAge: number;
}>;
export declare function sendResponse(req: IncomingMessage, res: ServerResponse, url: string, extension: string, buffer: Buffer, isStatic: boolean, xCache: XCacheHeader, imagesConfig: ImageConfigComplete, maxAge: number, isDev: boolean): void;
export declare function getImageSize(buffer: Buffer, extension: 'avif' | 'webp' | 'png' | 'jpeg'): Promise<{
    width?: number;
    height?: number;
}>;
export {};
