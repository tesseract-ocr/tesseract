"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    default: null,
    raw: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    default: function() {
        return _default;
    },
    raw: function() {
        return raw;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _loaderutils3 = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/loader-utils3"));
const _imageoptimizer = require("../../../../server/image-optimizer");
const _blur = require("./blur");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function nextImageLoader(content) {
    const imageLoaderSpan = this.currentTraceSpan.traceChild('next-image-loader');
    return imageLoaderSpan.traceAsyncFn(async ()=>{
        const options = this.getOptions();
        const { compilerType, isDev, assetPrefix, basePath } = options;
        const context = this.rootContext;
        const opts = {
            context,
            content
        };
        const interpolatedName = _loaderutils3.default.interpolateName(this, '/static/media/[name].[hash:8].[ext]', opts);
        const outputPath = assetPrefix + '/_next' + interpolatedName;
        let extension = _loaderutils3.default.interpolateName(this, '[ext]', opts);
        if (extension === 'jpg') {
            extension = 'jpeg';
        }
        const imageSizeSpan = imageLoaderSpan.traceChild('image-size-calculation');
        const imageSize = await imageSizeSpan.traceAsyncFn(()=>(0, _imageoptimizer.getImageSize)(content).catch((err)=>err));
        if (imageSize instanceof Error) {
            const err = imageSize;
            err.name = 'InvalidImageFormatError';
            throw err;
        }
        const { dataURL: blurDataURL, width: blurWidth, height: blurHeight } = await (0, _blur.getBlurImage)(content, extension, imageSize, {
            basePath,
            outputPath,
            isDev,
            tracing: imageLoaderSpan.traceChild.bind(imageLoaderSpan)
        });
        const stringifiedData = imageLoaderSpan.traceChild('image-data-stringify').traceFn(()=>JSON.stringify({
                src: outputPath,
                height: imageSize.height,
                width: imageSize.width,
                blurDataURL,
                blurWidth,
                blurHeight
            }));
        if (compilerType === 'client') {
            this.emitFile(interpolatedName, content, null);
        } else {
            this.emitFile(_path.default.join('..', isDev || compilerType === 'edge-server' ? '' : '..', interpolatedName), content, null);
        }
        return `export default ${stringifiedData};`;
    });
}
const raw = true;
const _default = nextImageLoader;

//# sourceMappingURL=index.js.map