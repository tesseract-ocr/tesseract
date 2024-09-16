"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    decodeBuffer: null,
    encodeAvif: null,
    encodeJpeg: null,
    encodePng: null,
    encodeWebp: null,
    resize: null,
    rotate: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    decodeBuffer: function() {
        return decodeBuffer;
    },
    encodeAvif: function() {
        return encodeAvif;
    },
    encodeJpeg: function() {
        return encodeJpeg;
    },
    encodePng: function() {
        return encodePng;
    },
    encodeWebp: function() {
        return encodeWebp;
    },
    resize: function() {
        return resize;
    },
    rotate: function() {
        return rotate;
    }
});
const _codecs = require("./codecs");
const _image_data = /*#__PURE__*/ _interop_require_default(require("./image_data"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function decodeBuffer(_buffer) {
    var _Object_entries_find;
    const buffer = Buffer.from(_buffer);
    const firstChunk = buffer.slice(0, 16);
    const firstChunkString = Array.from(firstChunk).map((v)=>String.fromCodePoint(v)).join("");
    const key = (_Object_entries_find = Object.entries(_codecs.codecs).find(([, { detectors }])=>detectors.some((detector)=>detector.exec(firstChunkString)))) == null ? void 0 : _Object_entries_find[0];
    if (!key) {
        throw Error(`Buffer has an unsupported format`);
    }
    const encoder = _codecs.codecs[key];
    const mod = await encoder.dec();
    const rgba = mod.decode(new Uint8Array(buffer));
    return rgba;
}
async function rotate(image, numRotations) {
    image = _image_data.default.from(image);
    const m = await _codecs.preprocessors["rotate"].instantiate();
    return await m(image.data, image.width, image.height, {
        numRotations
    });
}
async function resize({ image, width, height }) {
    image = _image_data.default.from(image);
    const p = _codecs.preprocessors["resize"];
    const m = await p.instantiate();
    return await m(image.data, image.width, image.height, {
        ...p.defaultOptions,
        width,
        height
    });
}
async function encodeJpeg(image, { quality }) {
    image = _image_data.default.from(image);
    const e = _codecs.codecs["mozjpeg"];
    const m = await e.enc();
    const r = await m.encode(image.data, image.width, image.height, {
        ...e.defaultEncoderOptions,
        quality
    });
    return Buffer.from(r);
}
async function encodeWebp(image, { quality }) {
    image = _image_data.default.from(image);
    const e = _codecs.codecs["webp"];
    const m = await e.enc();
    const r = await m.encode(image.data, image.width, image.height, {
        ...e.defaultEncoderOptions,
        quality
    });
    return Buffer.from(r);
}
async function encodeAvif(image, { quality }) {
    image = _image_data.default.from(image);
    const e = _codecs.codecs["avif"];
    const m = await e.enc();
    const val = e.autoOptimize.min || 62;
    const r = await m.encode(image.data, image.width, image.height, {
        ...e.defaultEncoderOptions,
        // Think of cqLevel as the "amount" of quantization (0 to 62),
        // so a lower value yields higher quality (0 to 100).
        cqLevel: Math.round(val - quality / 100 * val)
    });
    return Buffer.from(r);
}
async function encodePng(image) {
    image = _image_data.default.from(image);
    const e = _codecs.codecs["oxipng"];
    const m = await e.enc();
    const r = await m.encode(image.data, image.width, image.height, {
        ...e.defaultEncoderOptions
    });
    return Buffer.from(r);
}

//# sourceMappingURL=impl.js.map