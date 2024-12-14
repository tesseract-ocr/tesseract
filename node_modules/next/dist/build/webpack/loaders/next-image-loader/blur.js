"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getBlurImage", {
    enumerable: true,
    get: function() {
        return getBlurImage;
    }
});
const _isanimated = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/is-animated"));
const _imageoptimizer = require("../../../../server/image-optimizer");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const BLUR_IMG_SIZE = 8;
const BLUR_QUALITY = 70;
const VALID_BLUR_EXT = [
    'jpeg',
    'png',
    'webp',
    'avif'
] // should match other usages
;
async function getBlurImage(content, extension, imageSize, { basePath, outputPath, isDev, tracing = ()=>({
        traceFn: (fn)=>(...args)=>fn(...args),
        traceAsyncFn: (fn)=>(...args)=>fn(...args)
    }) }) {
    let blurDataURL;
    let blurWidth = 0;
    let blurHeight = 0;
    if (VALID_BLUR_EXT.includes(extension) && !(0, _isanimated.default)(content)) {
        // Shrink the image's largest dimension
        if (imageSize.width >= imageSize.height) {
            blurWidth = BLUR_IMG_SIZE;
            blurHeight = Math.max(Math.round(imageSize.height / imageSize.width * BLUR_IMG_SIZE), 1);
        } else {
            blurWidth = Math.max(Math.round(imageSize.width / imageSize.height * BLUR_IMG_SIZE), 1);
            blurHeight = BLUR_IMG_SIZE;
        }
        if (isDev) {
            // During `next dev`, we don't want to generate blur placeholders with webpack
            // because it can delay starting the dev server. Instead, we inline a
            // special url to lazily generate the blur placeholder at request time.
            const prefix = 'http://localhost';
            const url = new URL(`${basePath || ''}/_next/image`, prefix);
            url.searchParams.set('url', outputPath);
            url.searchParams.set('w', String(blurWidth));
            url.searchParams.set('q', String(BLUR_QUALITY));
            blurDataURL = url.href.slice(prefix.length);
        } else {
            const resizeImageSpan = tracing('image-resize');
            const resizedImage = await resizeImageSpan.traceAsyncFn(()=>(0, _imageoptimizer.optimizeImage)({
                    buffer: content,
                    width: blurWidth,
                    height: blurHeight,
                    contentType: `image/${extension}`,
                    quality: BLUR_QUALITY
                }));
            const blurDataURLSpan = tracing('image-base64-tostring');
            blurDataURL = blurDataURLSpan.traceFn(()=>`data:image/${extension};base64,${resizedImage.toString('base64')}`);
        }
    }
    return {
        dataURL: blurDataURL,
        width: blurWidth,
        height: blurHeight
    };
}

//# sourceMappingURL=blur.js.map