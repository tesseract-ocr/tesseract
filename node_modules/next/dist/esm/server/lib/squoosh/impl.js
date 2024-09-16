import { codecs as supportedFormats, preprocessors } from "./codecs";
import ImageData from "./image_data";
export async function decodeBuffer(_buffer) {
    var _Object_entries_find;
    const buffer = Buffer.from(_buffer);
    const firstChunk = buffer.slice(0, 16);
    const firstChunkString = Array.from(firstChunk).map((v)=>String.fromCodePoint(v)).join("");
    const key = (_Object_entries_find = Object.entries(supportedFormats).find(([, { detectors }])=>detectors.some((detector)=>detector.exec(firstChunkString)))) == null ? void 0 : _Object_entries_find[0];
    if (!key) {
        throw Error(`Buffer has an unsupported format`);
    }
    const encoder = supportedFormats[key];
    const mod = await encoder.dec();
    const rgba = mod.decode(new Uint8Array(buffer));
    return rgba;
}
export async function rotate(image, numRotations) {
    image = ImageData.from(image);
    const m = await preprocessors["rotate"].instantiate();
    return await m(image.data, image.width, image.height, {
        numRotations
    });
}
export async function resize({ image, width, height }) {
    image = ImageData.from(image);
    const p = preprocessors["resize"];
    const m = await p.instantiate();
    return await m(image.data, image.width, image.height, {
        ...p.defaultOptions,
        width,
        height
    });
}
export async function encodeJpeg(image, { quality }) {
    image = ImageData.from(image);
    const e = supportedFormats["mozjpeg"];
    const m = await e.enc();
    const r = await m.encode(image.data, image.width, image.height, {
        ...e.defaultEncoderOptions,
        quality
    });
    return Buffer.from(r);
}
export async function encodeWebp(image, { quality }) {
    image = ImageData.from(image);
    const e = supportedFormats["webp"];
    const m = await e.enc();
    const r = await m.encode(image.data, image.width, image.height, {
        ...e.defaultEncoderOptions,
        quality
    });
    return Buffer.from(r);
}
export async function encodeAvif(image, { quality }) {
    image = ImageData.from(image);
    const e = supportedFormats["avif"];
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
export async function encodePng(image) {
    image = ImageData.from(image);
    const e = supportedFormats["oxipng"];
    const m = await e.enc();
    const r = await m.encode(image.data, image.width, image.height, {
        ...e.defaultEncoderOptions
    });
    return Buffer.from(r);
}

//# sourceMappingURL=impl.js.map