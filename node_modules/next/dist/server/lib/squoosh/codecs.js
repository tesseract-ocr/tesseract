"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    codecs: null,
    preprocessors: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    codecs: function() {
        return codecs;
    },
    preprocessors: function() {
        return preprocessors;
    }
});
const _fs = require("fs");
const _path = /*#__PURE__*/ _interop_require_wildcard(require("path"));
const _emscriptenutils = require("./emscripten-utils.js");
const _mozjpeg_node_enc = /*#__PURE__*/ _interop_require_default(require("./mozjpeg/mozjpeg_node_enc.js"));
const _mozjpeg_node_dec = /*#__PURE__*/ _interop_require_default(require("./mozjpeg/mozjpeg_node_dec.js"));
const _webp_node_enc = /*#__PURE__*/ _interop_require_default(require("./webp/webp_node_enc.js"));
const _webp_node_dec = /*#__PURE__*/ _interop_require_default(require("./webp/webp_node_dec.js"));
const _avif_node_enc = /*#__PURE__*/ _interop_require_default(require("./avif/avif_node_enc.js"));
const _avif_node_dec = /*#__PURE__*/ _interop_require_default(require("./avif/avif_node_dec.js"));
const _squoosh_png = /*#__PURE__*/ _interop_require_wildcard(require("./png/squoosh_png.js"));
const _squoosh_oxipng = /*#__PURE__*/ _interop_require_wildcard(require("./png/squoosh_oxipng.js"));
const _squoosh_resize = /*#__PURE__*/ _interop_require_wildcard(require("./resize/squoosh_resize.js"));
const _image_data = /*#__PURE__*/ _interop_require_default(require("./image_data"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
const mozEncWasm = _path.resolve(__dirname, "./mozjpeg/mozjpeg_node_enc.wasm");
const mozDecWasm = _path.resolve(__dirname, "./mozjpeg/mozjpeg_node_dec.wasm");
const webpEncWasm = _path.resolve(__dirname, "./webp/webp_node_enc.wasm");
const webpDecWasm = _path.resolve(__dirname, "./webp/webp_node_dec.wasm");
const avifEncWasm = _path.resolve(__dirname, "./avif/avif_node_enc.wasm");
const avifDecWasm = _path.resolve(__dirname, "./avif/avif_node_dec.wasm");
const pngEncDecWasm = _path.resolve(__dirname, "./png/squoosh_png_bg.wasm");
const pngEncDecInit = ()=>_squoosh_png.default(_fs.promises.readFile((0, _emscriptenutils.pathify)(pngEncDecWasm)));
const oxipngWasm = _path.resolve(__dirname, "./png/squoosh_oxipng_bg.wasm");
const oxipngInit = ()=>_squoosh_oxipng.default(_fs.promises.readFile((0, _emscriptenutils.pathify)(oxipngWasm)));
const resizeWasm = _path.resolve(__dirname, "./resize/squoosh_resize_bg.wasm");
const resizeInit = ()=>_squoosh_resize.default(_fs.promises.readFile((0, _emscriptenutils.pathify)(resizeWasm)));
// rotate
const rotateWasm = _path.resolve(__dirname, "./rotate/rotate.wasm");
globalThis.ImageData = _image_data.default;
function resizeNameToIndex(name) {
    switch(name){
        case "triangle":
            return 0;
        case "catrom":
            return 1;
        case "mitchell":
            return 2;
        case "lanczos3":
            return 3;
        default:
            throw Error(`Unknown resize algorithm "${name}"`);
    }
}
function resizeWithAspect({ input_width, input_height, target_width, target_height }) {
    if (!target_width && !target_height) {
        throw Error("Need to specify at least width or height when resizing");
    }
    if (target_width && target_height) {
        return {
            width: target_width,
            height: target_height
        };
    }
    if (!target_width) {
        return {
            width: Math.round(input_width / input_height * target_height),
            height: target_height
        };
    }
    return {
        width: target_width,
        height: Math.round(input_height / input_width * target_width)
    };
}
const preprocessors = {
    resize: {
        name: "Resize",
        description: "Resize the image before compressing",
        instantiate: async ()=>{
            await resizeInit();
            return (buffer, input_width, input_height, { width, height, method, premultiply, linearRGB })=>{
                ({ width, height } = resizeWithAspect({
                    input_width,
                    input_height,
                    target_width: width,
                    target_height: height
                }));
                const imageData = new _image_data.default(_squoosh_resize.resize(buffer, input_width, input_height, width, height, resizeNameToIndex(method), premultiply, linearRGB), width, height);
                _squoosh_resize.cleanup();
                return imageData;
            };
        },
        defaultOptions: {
            method: "lanczos3",
            fitMethod: "stretch",
            premultiply: true,
            linearRGB: true
        }
    },
    rotate: {
        name: "Rotate",
        description: "Rotate image",
        instantiate: async ()=>{
            return async (buffer, width, height, { numRotations })=>{
                const degrees = numRotations * 90 % 360;
                const sameDimensions = degrees === 0 || degrees === 180;
                const size = width * height * 4;
                const instance = (await WebAssembly.instantiate(await _fs.promises.readFile((0, _emscriptenutils.pathify)(rotateWasm)))).instance;
                const { memory } = instance.exports;
                const additionalPagesNeeded = Math.ceil((size * 2 - memory.buffer.byteLength + 8) / (64 * 1024));
                if (additionalPagesNeeded > 0) {
                    memory.grow(additionalPagesNeeded);
                }
                const view = new Uint8ClampedArray(memory.buffer);
                view.set(buffer, 8);
                instance.exports.rotate(width, height, degrees);
                return new _image_data.default(view.slice(size + 8, size * 2 + 8), sameDimensions ? width : height, sameDimensions ? height : width);
            };
        },
        defaultOptions: {
            numRotations: 0
        }
    }
};
const codecs = {
    mozjpeg: {
        name: "MozJPEG",
        extension: "jpg",
        detectors: [
            /^\xFF\xD8\xFF/
        ],
        dec: ()=>(0, _emscriptenutils.instantiateEmscriptenWasm)(_mozjpeg_node_dec.default, mozDecWasm),
        enc: ()=>(0, _emscriptenutils.instantiateEmscriptenWasm)(_mozjpeg_node_enc.default, mozEncWasm),
        defaultEncoderOptions: {
            quality: 75,
            baseline: false,
            arithmetic: false,
            progressive: true,
            optimize_coding: true,
            smoothing: 0,
            color_space: 3 /*YCbCr*/ ,
            quant_table: 3,
            trellis_multipass: false,
            trellis_opt_zero: false,
            trellis_opt_table: false,
            trellis_loops: 1,
            auto_subsample: true,
            chroma_subsample: 2,
            separate_chroma_quality: false,
            chroma_quality: 75
        },
        autoOptimize: {
            option: "quality",
            min: 0,
            max: 100
        }
    },
    webp: {
        name: "WebP",
        extension: "webp",
        detectors: [
            /^RIFF....WEBPVP8[LX ]/s
        ],
        dec: ()=>(0, _emscriptenutils.instantiateEmscriptenWasm)(_webp_node_dec.default, webpDecWasm),
        enc: ()=>(0, _emscriptenutils.instantiateEmscriptenWasm)(_webp_node_enc.default, webpEncWasm),
        defaultEncoderOptions: {
            quality: 75,
            target_size: 0,
            target_PSNR: 0,
            method: 4,
            sns_strength: 50,
            filter_strength: 60,
            filter_sharpness: 0,
            filter_type: 1,
            partitions: 0,
            segments: 4,
            pass: 1,
            show_compressed: 0,
            preprocessing: 0,
            autofilter: 0,
            partition_limit: 0,
            alpha_compression: 1,
            alpha_filtering: 1,
            alpha_quality: 100,
            lossless: 0,
            exact: 0,
            image_hint: 0,
            emulate_jpeg_size: 0,
            thread_level: 0,
            low_memory: 0,
            near_lossless: 100,
            use_delta_palette: 0,
            use_sharp_yuv: 0
        },
        autoOptimize: {
            option: "quality",
            min: 0,
            max: 100
        }
    },
    avif: {
        name: "AVIF",
        extension: "avif",
        // eslint-disable-next-line no-control-regex
        detectors: [
            /^\x00\x00\x00 ftypavif\x00\x00\x00\x00/
        ],
        dec: ()=>(0, _emscriptenutils.instantiateEmscriptenWasm)(_avif_node_dec.default, avifDecWasm),
        enc: async ()=>{
            return (0, _emscriptenutils.instantiateEmscriptenWasm)(_avif_node_enc.default, avifEncWasm);
        },
        defaultEncoderOptions: {
            cqLevel: 33,
            cqAlphaLevel: -1,
            denoiseLevel: 0,
            tileColsLog2: 0,
            tileRowsLog2: 0,
            speed: 6,
            subsample: 1,
            chromaDeltaQ: false,
            sharpness: 0,
            tune: 0 /* AVIFTune.auto */ 
        },
        autoOptimize: {
            option: "cqLevel",
            min: 62,
            max: 0
        }
    },
    oxipng: {
        name: "OxiPNG",
        extension: "png",
        // eslint-disable-next-line no-control-regex
        detectors: [
            /^\x89PNG\x0D\x0A\x1A\x0A/
        ],
        dec: async ()=>{
            await pngEncDecInit();
            return {
                decode: (buffer)=>{
                    const imageData = _squoosh_png.decode(buffer);
                    _squoosh_png.cleanup();
                    return imageData;
                }
            };
        },
        enc: async ()=>{
            await pngEncDecInit();
            await oxipngInit();
            return {
                encode: (buffer, width, height, opts)=>{
                    const simplePng = _squoosh_png.encode(new Uint8Array(buffer), width, height);
                    const imageData = _squoosh_oxipng.optimise(simplePng, opts.level, false);
                    _squoosh_oxipng.cleanup();
                    return imageData;
                }
            };
        },
        defaultEncoderOptions: {
            level: 2
        },
        autoOptimize: {
            option: "level",
            min: 6,
            max: 1
        }
    }
};

//# sourceMappingURL=codecs.js.map