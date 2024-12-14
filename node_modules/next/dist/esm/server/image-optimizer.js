import { createHash } from 'crypto';
import { promises } from 'fs';
import { mediaType } from 'next/dist/compiled/@hapi/accept';
import contentDisposition from 'next/dist/compiled/content-disposition';
import imageSizeOf from 'next/dist/compiled/image-size';
import isAnimated from 'next/dist/compiled/is-animated';
import { join } from 'path';
import nodeUrl from 'url';
import { getImageBlurSvg } from '../shared/lib/image-blur-svg';
import { hasLocalMatch } from '../shared/lib/match-local-pattern';
import { hasRemoteMatch } from '../shared/lib/match-remote-pattern';
import { createRequestResponseMocks } from './lib/mock-request';
import { CachedRouteKind } from './response-cache';
import { sendEtagResponse } from './send-payload';
import { getContentType, getExtension } from './serve-static';
import * as Log from '../build/output/log';
import isError from '../lib/is-error';
import { parseUrl } from '../lib/url';
const AVIF = 'image/avif';
const WEBP = 'image/webp';
const PNG = 'image/png';
const JPEG = 'image/jpeg';
const GIF = 'image/gif';
const SVG = 'image/svg+xml';
const ICO = 'image/x-icon';
const TIFF = 'image/tiff';
const BMP = 'image/bmp';
const CACHE_VERSION = 4;
const ANIMATABLE_TYPES = [
    WEBP,
    PNG,
    GIF
];
const VECTOR_TYPES = [
    SVG
];
const BLUR_IMG_SIZE = 8 // should match `next-image-loader`
;
const BLUR_QUALITY = 70 // should match `next-image-loader`
;
let _sharp;
export function getSharp(concurrency) {
    if (_sharp) {
        return _sharp;
    }
    try {
        _sharp = require('sharp');
        if (_sharp && _sharp.concurrency() > 1) {
            // Reducing concurrency should reduce the memory usage too.
            // We more aggressively reduce in dev but also reduce in prod.
            // https://sharp.pixelplumbing.com/api-utility#concurrency
            const divisor = process.env.NODE_ENV === 'development' ? 4 : 2;
            _sharp.concurrency(concurrency ?? Math.floor(Math.max(_sharp.concurrency() / divisor, 1)));
        }
    } catch (e) {
        if (isError(e) && e.code === 'MODULE_NOT_FOUND') {
            throw new Error('Module `sharp` not found. Please run `npm install --cpu=wasm32 sharp` to install it.');
        }
        throw e;
    }
    return _sharp;
}
function getSupportedMimeType(options, accept = '') {
    const mimeType = mediaType(accept, options);
    return accept.includes(mimeType) ? mimeType : '';
}
export function getHash(items) {
    const hash = createHash('sha256');
    for (let item of items){
        if (typeof item === 'number') hash.update(String(item));
        else {
            hash.update(item);
        }
    }
    // See https://en.wikipedia.org/wiki/Base64#URL_applications
    return hash.digest('base64url');
}
export function extractEtag(etag, imageBuffer) {
    if (etag) {
        // upstream etag needs to be base64url encoded due to weak etag signature
        // as we store this in the cache-entry file name.
        return Buffer.from(etag).toString('base64url');
    }
    return getImageEtag(imageBuffer);
}
export function getImageEtag(image) {
    return getHash([
        image
    ]);
}
async function writeToCacheDir(dir, extension, maxAge, expireAt, buffer, etag, upstreamEtag) {
    const filename = join(dir, `${maxAge}.${expireAt}.${etag}.${upstreamEtag}.${extension}`);
    await promises.rm(dir, {
        recursive: true,
        force: true
    }).catch(()=>{});
    await promises.mkdir(dir, {
        recursive: true
    });
    await promises.writeFile(filename, buffer);
}
/**
 * Inspects the first few bytes of a buffer to determine if
 * it matches the "magic number" of known file signatures.
 * https://en.wikipedia.org/wiki/List_of_file_signatures
 */ export function detectContentType(buffer) {
    if ([
        0xff,
        0xd8,
        0xff
    ].every((b, i)=>buffer[i] === b)) {
        return JPEG;
    }
    if ([
        0x89,
        0x50,
        0x4e,
        0x47,
        0x0d,
        0x0a,
        0x1a,
        0x0a
    ].every((b, i)=>buffer[i] === b)) {
        return PNG;
    }
    if ([
        0x47,
        0x49,
        0x46,
        0x38
    ].every((b, i)=>buffer[i] === b)) {
        return GIF;
    }
    if ([
        0x52,
        0x49,
        0x46,
        0x46,
        0,
        0,
        0,
        0,
        0x57,
        0x45,
        0x42,
        0x50
    ].every((b, i)=>!b || buffer[i] === b)) {
        return WEBP;
    }
    if ([
        0x3c,
        0x3f,
        0x78,
        0x6d,
        0x6c
    ].every((b, i)=>buffer[i] === b)) {
        return SVG;
    }
    if ([
        0x3c,
        0x73,
        0x76,
        0x67
    ].every((b, i)=>buffer[i] === b)) {
        return SVG;
    }
    if ([
        0,
        0,
        0,
        0,
        0x66,
        0x74,
        0x79,
        0x70,
        0x61,
        0x76,
        0x69,
        0x66
    ].every((b, i)=>!b || buffer[i] === b)) {
        return AVIF;
    }
    if ([
        0x00,
        0x00,
        0x01,
        0x00
    ].every((b, i)=>buffer[i] === b)) {
        return ICO;
    }
    if ([
        0x49,
        0x49,
        0x2a,
        0x00
    ].every((b, i)=>buffer[i] === b)) {
        return TIFF;
    }
    if ([
        0x42,
        0x4d
    ].every((b, i)=>buffer[i] === b)) {
        return BMP;
    }
    return null;
}
export class ImageOptimizerCache {
    static validateParams(req, query, nextConfig, isDev) {
        var _nextConfig_images, _nextConfig_images1;
        const imageData = nextConfig.images;
        const { deviceSizes = [], imageSizes = [], domains = [], minimumCacheTTL = 60, formats = [
            'image/webp'
        ] } = imageData;
        const remotePatterns = ((_nextConfig_images = nextConfig.images) == null ? void 0 : _nextConfig_images.remotePatterns) || [];
        const localPatterns = (_nextConfig_images1 = nextConfig.images) == null ? void 0 : _nextConfig_images1.localPatterns;
        const { url, w, q } = query;
        let href;
        if (domains.length > 0) {
            Log.warnOnce('The "images.domains" configuration is deprecated. Please use "images.remotePatterns" configuration instead.');
        }
        if (!url) {
            return {
                errorMessage: '"url" parameter is required'
            };
        } else if (Array.isArray(url)) {
            return {
                errorMessage: '"url" parameter cannot be an array'
            };
        }
        if (url.length > 3072) {
            return {
                errorMessage: '"url" parameter is too long'
            };
        }
        if (url.startsWith('//')) {
            return {
                errorMessage: '"url" parameter cannot be a protocol-relative URL (//)'
            };
        }
        let isAbsolute;
        if (url.startsWith('/')) {
            var _parseUrl;
            href = url;
            isAbsolute = false;
            if (/\/_next\/image($|\/)/.test(decodeURIComponent(((_parseUrl = parseUrl(url)) == null ? void 0 : _parseUrl.pathname) ?? ''))) {
                return {
                    errorMessage: '"url" parameter cannot be recursive'
                };
            }
            if (!hasLocalMatch(localPatterns, url)) {
                return {
                    errorMessage: '"url" parameter is not allowed'
                };
            }
        } else {
            let hrefParsed;
            try {
                hrefParsed = new URL(url);
                href = hrefParsed.toString();
                isAbsolute = true;
            } catch (_error) {
                return {
                    errorMessage: '"url" parameter is invalid'
                };
            }
            if (![
                'http:',
                'https:'
            ].includes(hrefParsed.protocol)) {
                return {
                    errorMessage: '"url" parameter is invalid'
                };
            }
            if (!hasRemoteMatch(domains, remotePatterns, hrefParsed)) {
                return {
                    errorMessage: '"url" parameter is not allowed'
                };
            }
        }
        if (!w) {
            return {
                errorMessage: '"w" parameter (width) is required'
            };
        } else if (Array.isArray(w)) {
            return {
                errorMessage: '"w" parameter (width) cannot be an array'
            };
        } else if (!/^[0-9]+$/.test(w)) {
            return {
                errorMessage: '"w" parameter (width) must be an integer greater than 0'
            };
        }
        if (!q) {
            return {
                errorMessage: '"q" parameter (quality) is required'
            };
        } else if (Array.isArray(q)) {
            return {
                errorMessage: '"q" parameter (quality) cannot be an array'
            };
        } else if (!/^[0-9]+$/.test(q)) {
            return {
                errorMessage: '"q" parameter (quality) must be an integer between 1 and 100'
            };
        }
        const width = parseInt(w, 10);
        if (width <= 0 || isNaN(width)) {
            return {
                errorMessage: '"w" parameter (width) must be an integer greater than 0'
            };
        }
        const sizes = [
            ...deviceSizes || [],
            ...imageSizes || []
        ];
        if (isDev) {
            sizes.push(BLUR_IMG_SIZE);
        }
        const isValidSize = sizes.includes(width) || isDev && width <= BLUR_IMG_SIZE;
        if (!isValidSize) {
            return {
                errorMessage: `"w" parameter (width) of ${width} is not allowed`
            };
        }
        const quality = parseInt(q, 10);
        if (isNaN(quality) || quality < 1 || quality > 100) {
            return {
                errorMessage: '"q" parameter (quality) must be an integer between 1 and 100'
            };
        }
        const mimeType = getSupportedMimeType(formats || [], req.headers['accept']);
        const isStatic = url.startsWith(`${nextConfig.basePath || ''}/_next/static/media`);
        return {
            href,
            sizes,
            isAbsolute,
            isStatic,
            width,
            quality,
            mimeType,
            minimumCacheTTL
        };
    }
    static getCacheKey({ href, width, quality, mimeType }) {
        return getHash([
            CACHE_VERSION,
            href,
            width,
            quality,
            mimeType
        ]);
    }
    constructor({ distDir, nextConfig }){
        this.cacheDir = join(distDir, 'cache', 'images');
        this.nextConfig = nextConfig;
    }
    async get(cacheKey) {
        try {
            const cacheDir = join(this.cacheDir, cacheKey);
            const files = await promises.readdir(cacheDir);
            const now = Date.now();
            for (const file of files){
                const [maxAgeSt, expireAtSt, etag, upstreamEtag, extension] = file.split('.', 5);
                const buffer = await promises.readFile(join(cacheDir, file));
                const expireAt = Number(expireAtSt);
                const maxAge = Number(maxAgeSt);
                return {
                    value: {
                        kind: CachedRouteKind.IMAGE,
                        etag,
                        buffer,
                        extension,
                        upstreamEtag
                    },
                    revalidateAfter: Math.max(maxAge, this.nextConfig.images.minimumCacheTTL) * 1000 + Date.now(),
                    curRevalidate: maxAge,
                    isStale: now > expireAt,
                    isFallback: false
                };
            }
        } catch (_) {
        // failed to read from cache dir, treat as cache miss
        }
        return null;
    }
    async set(cacheKey, value, { revalidate }) {
        if ((value == null ? void 0 : value.kind) !== CachedRouteKind.IMAGE) {
            throw new Error('invariant attempted to set non-image to image-cache');
        }
        if (typeof revalidate !== 'number') {
            throw new Error('invariant revalidate must be a number for image-cache');
        }
        const expireAt = Math.max(revalidate, this.nextConfig.images.minimumCacheTTL) * 1000 + Date.now();
        try {
            await writeToCacheDir(join(this.cacheDir, cacheKey), value.extension, revalidate, expireAt, value.buffer, value.etag, value.upstreamEtag);
        } catch (err) {
            Log.error(`Failed to write image to cache ${cacheKey}`, err);
        }
    }
}
export class ImageError extends Error {
    constructor(statusCode, message){
        super(message);
        // ensure an error status is used > 400
        if (statusCode >= 400) {
            this.statusCode = statusCode;
        } else {
            this.statusCode = 500;
        }
    }
}
function parseCacheControl(str) {
    const map = new Map();
    if (!str) {
        return map;
    }
    for (let directive of str.split(',')){
        let [key, value] = directive.trim().split('=', 2);
        key = key.toLowerCase();
        if (value) {
            value = value.toLowerCase();
        }
        map.set(key, value);
    }
    return map;
}
export function getMaxAge(str) {
    const map = parseCacheControl(str);
    if (map) {
        let age = map.get('s-maxage') || map.get('max-age') || '';
        if (age.startsWith('"') && age.endsWith('"')) {
            age = age.slice(1, -1);
        }
        const n = parseInt(age, 10);
        if (!isNaN(n)) {
            return n;
        }
    }
    return 0;
}
export function getPreviouslyCachedImageOrNull(upstreamImage, previousCacheEntry) {
    var _previousCacheEntry_value;
    if ((previousCacheEntry == null ? void 0 : (_previousCacheEntry_value = previousCacheEntry.value) == null ? void 0 : _previousCacheEntry_value.kind) === 'IMAGE' && // Images that are SVGs, animated or failed the optimization previously end up using upstreamEtag as their etag as well,
    // in these cases we want to trigger a new "optimization" attempt.
    previousCacheEntry.value.upstreamEtag !== previousCacheEntry.value.etag && // and the upstream etag is the same as the previous cache entry's
    upstreamImage.etag === previousCacheEntry.value.upstreamEtag) {
        return previousCacheEntry.value;
    }
    return null;
}
export async function optimizeImage({ buffer, contentType, quality, width, height, concurrency, limitInputPixels, sequentialRead, timeoutInSeconds }) {
    const sharp = getSharp(concurrency);
    const transformer = sharp(buffer, {
        limitInputPixels,
        sequentialRead: sequentialRead ?? undefined
    }).timeout({
        seconds: timeoutInSeconds ?? 7
    }).rotate();
    if (height) {
        transformer.resize(width, height);
    } else {
        transformer.resize(width, undefined, {
            withoutEnlargement: true
        });
    }
    if (contentType === AVIF) {
        transformer.avif({
            quality: Math.max(quality - 20, 1),
            effort: 3
        });
    } else if (contentType === WEBP) {
        transformer.webp({
            quality
        });
    } else if (contentType === PNG) {
        transformer.png({
            quality
        });
    } else if (contentType === JPEG) {
        transformer.jpeg({
            quality,
            mozjpeg: true
        });
    }
    const optimizedBuffer = await transformer.toBuffer();
    return optimizedBuffer;
}
export async function fetchExternalImage(href) {
    const res = await fetch(href, {
        signal: AbortSignal.timeout(7000)
    }).catch((err)=>err);
    if (res instanceof Error) {
        const err = res;
        if (err.name === 'TimeoutError') {
            Log.error('upstream image response timed out for', href);
            throw new ImageError(504, '"url" parameter is valid but upstream response timed out');
        }
        throw err;
    }
    if (!res.ok) {
        Log.error('upstream image response failed for', href, res.status);
        throw new ImageError(res.status, '"url" parameter is valid but upstream response is invalid');
    }
    const buffer = Buffer.from(await res.arrayBuffer());
    const contentType = res.headers.get('Content-Type');
    const cacheControl = res.headers.get('Cache-Control');
    const etag = extractEtag(res.headers.get('ETag'), buffer);
    return {
        buffer,
        contentType,
        cacheControl,
        etag
    };
}
export async function fetchInternalImage(href, _req, _res, handleRequest) {
    try {
        const mocked = createRequestResponseMocks({
            url: href,
            method: _req.method || 'GET',
            headers: _req.headers,
            socket: _req.socket
        });
        await handleRequest(mocked.req, mocked.res, nodeUrl.parse(href, true));
        await mocked.res.hasStreamed;
        if (!mocked.res.statusCode) {
            Log.error('image response failed for', href, mocked.res.statusCode);
            throw new ImageError(mocked.res.statusCode, '"url" parameter is valid but internal response is invalid');
        }
        const buffer = Buffer.concat(mocked.res.buffers);
        const contentType = mocked.res.getHeader('Content-Type');
        const cacheControl = mocked.res.getHeader('Cache-Control');
        const etag = extractEtag(mocked.res.getHeader('ETag'), buffer);
        return {
            buffer,
            contentType,
            cacheControl,
            etag
        };
    } catch (err) {
        Log.error('upstream image response failed for', href, err);
        throw new ImageError(500, '"url" parameter is valid but upstream response is invalid');
    }
}
export async function imageOptimizer(imageUpstream, paramsResult, nextConfig, opts) {
    var _imageUpstream_contentType;
    const { href, quality, width, mimeType } = paramsResult;
    const { buffer: upstreamBuffer, etag: upstreamEtag } = imageUpstream;
    const maxAge = getMaxAge(imageUpstream.cacheControl);
    const upstreamType = detectContentType(upstreamBuffer) || ((_imageUpstream_contentType = imageUpstream.contentType) == null ? void 0 : _imageUpstream_contentType.toLowerCase().trim());
    if (upstreamType) {
        if (upstreamType.startsWith('image/svg') && !nextConfig.images.dangerouslyAllowSVG) {
            if (!opts.silent) {
                Log.error(`The requested resource "${href}" has type "${upstreamType}" but dangerouslyAllowSVG is disabled`);
            }
            throw new ImageError(400, '"url" parameter is valid but image type is not allowed');
        }
        if (ANIMATABLE_TYPES.includes(upstreamType) && isAnimated(upstreamBuffer)) {
            if (!opts.silent) {
                Log.warnOnce(`The requested resource "${href}" is an animated image so it will not be optimized. Consider adding the "unoptimized" property to the <Image>.`);
            }
            return {
                buffer: upstreamBuffer,
                contentType: upstreamType,
                maxAge,
                etag: upstreamEtag,
                upstreamEtag
            };
        }
        if (VECTOR_TYPES.includes(upstreamType)) {
            // We don't warn here because we already know that "dangerouslyAllowSVG"
            // was enabled above, therefore the user explicitly opted in.
            // If we add more VECTOR_TYPES besides SVG, perhaps we could warn for those.
            return {
                buffer: upstreamBuffer,
                contentType: upstreamType,
                maxAge,
                etag: upstreamEtag,
                upstreamEtag
            };
        }
        if (!upstreamType.startsWith('image/') || upstreamType.includes(',')) {
            if (!opts.silent) {
                Log.error("The requested resource isn't a valid image for", href, 'received', upstreamType);
            }
            throw new ImageError(400, "The requested resource isn't a valid image.");
        }
    }
    let contentType;
    if (mimeType) {
        contentType = mimeType;
    } else if ((upstreamType == null ? void 0 : upstreamType.startsWith('image/')) && getExtension(upstreamType) && upstreamType !== WEBP && upstreamType !== AVIF) {
        contentType = upstreamType;
    } else {
        contentType = JPEG;
    }
    const previouslyCachedImage = getPreviouslyCachedImageOrNull(imageUpstream, opts.previousCacheEntry);
    if (previouslyCachedImage) {
        var _opts_previousCacheEntry;
        return {
            buffer: previouslyCachedImage.buffer,
            contentType,
            maxAge: (opts == null ? void 0 : (_opts_previousCacheEntry = opts.previousCacheEntry) == null ? void 0 : _opts_previousCacheEntry.curRevalidate) || maxAge,
            etag: previouslyCachedImage.etag,
            upstreamEtag: previouslyCachedImage.upstreamEtag
        };
    }
    try {
        let optimizedBuffer = await optimizeImage({
            buffer: upstreamBuffer,
            contentType,
            quality,
            width,
            concurrency: nextConfig.experimental.imgOptConcurrency,
            limitInputPixels: nextConfig.experimental.imgOptMaxInputPixels,
            sequentialRead: nextConfig.experimental.imgOptSequentialRead,
            timeoutInSeconds: nextConfig.experimental.imgOptTimeoutInSeconds
        });
        if (opts.isDev && width <= BLUR_IMG_SIZE && quality === BLUR_QUALITY) {
            // During `next dev`, we don't want to generate blur placeholders with webpack
            // because it can delay starting the dev server. Instead, `next-image-loader.js`
            // will inline a special url to lazily generate the blur placeholder at request time.
            const meta = await getImageSize(optimizedBuffer);
            const blurOpts = {
                blurWidth: meta.width,
                blurHeight: meta.height,
                blurDataURL: `data:${contentType};base64,${optimizedBuffer.toString('base64')}`
            };
            optimizedBuffer = Buffer.from(unescape(getImageBlurSvg(blurOpts)));
            contentType = 'image/svg+xml';
        }
        return {
            buffer: optimizedBuffer,
            contentType,
            maxAge: Math.max(maxAge, nextConfig.images.minimumCacheTTL),
            etag: getImageEtag(optimizedBuffer),
            upstreamEtag
        };
    } catch (error) {
        if (upstreamType) {
            // If we fail to optimize, fallback to the original image
            return {
                buffer: upstreamBuffer,
                contentType: upstreamType,
                maxAge: nextConfig.images.minimumCacheTTL,
                etag: upstreamEtag,
                upstreamEtag,
                error
            };
        } else {
            throw new ImageError(400, 'Unable to optimize image and unable to fallback to upstream image');
        }
    }
}
function getFileNameWithExtension(url, contentType) {
    const [urlWithoutQueryParams] = url.split('?', 1);
    const fileNameWithExtension = urlWithoutQueryParams.split('/').pop();
    if (!contentType || !fileNameWithExtension) {
        return 'image.bin';
    }
    const [fileName] = fileNameWithExtension.split('.', 1);
    const extension = getExtension(contentType);
    return `${fileName}.${extension}`;
}
function setResponseHeaders(req, res, url, etag, contentType, isStatic, xCache, imagesConfig, maxAge, isDev) {
    res.setHeader('Vary', 'Accept');
    res.setHeader('Cache-Control', isStatic ? 'public, max-age=315360000, immutable' : `public, max-age=${isDev ? 0 : maxAge}, must-revalidate`);
    if (sendEtagResponse(req, res, etag)) {
        // already called res.end() so we're finished
        return {
            finished: true
        };
    }
    if (contentType) {
        res.setHeader('Content-Type', contentType);
    }
    const fileName = getFileNameWithExtension(url, contentType);
    res.setHeader('Content-Disposition', contentDisposition(fileName, {
        type: imagesConfig.contentDispositionType
    }));
    res.setHeader('Content-Security-Policy', imagesConfig.contentSecurityPolicy);
    res.setHeader('X-Nextjs-Cache', xCache);
    return {
        finished: false
    };
}
export function sendResponse(req, res, url, extension, buffer, etag, isStatic, xCache, imagesConfig, maxAge, isDev) {
    const contentType = getContentType(extension);
    const result = setResponseHeaders(req, res, url, etag, contentType, isStatic, xCache, imagesConfig, maxAge, isDev);
    if (!result.finished) {
        res.setHeader('Content-Length', Buffer.byteLength(buffer));
        res.end(buffer);
    }
}
export async function getImageSize(buffer) {
    const { width, height } = imageSizeOf(buffer);
    return {
        width,
        height
    };
}

//# sourceMappingURL=image-optimizer.js.map