/// <reference types="node" />
interface DecodeModule extends EmscriptenWasm.Module {
    decode: (data: Uint8Array) => ImageData;
}
export interface ResizeOptions {
    width?: number;
    height?: number;
    method: 'triangle' | 'catrom' | 'mitchell' | 'lanczos3';
    premultiply: boolean;
    linearRGB: boolean;
}
export interface RotateOptions {
    numRotations: number;
}
import type { MozJPEGModule as MozJPEGEncodeModule } from './mozjpeg/mozjpeg_enc';
import type { WebPModule as WebPEncodeModule } from './webp/webp_enc';
import type { AVIFModule as AVIFEncodeModule } from './avif/avif_enc';
import ImageData from './image_data';
export declare const preprocessors: {
    readonly resize: {
        readonly name: "Resize";
        readonly description: "Resize the image before compressing";
        readonly instantiate: () => Promise<(buffer: Uint8Array, input_width: number, input_height: number, { width, height, method, premultiply, linearRGB }: ResizeOptions) => ImageData>;
        readonly defaultOptions: {
            readonly method: "lanczos3";
            readonly fitMethod: "stretch";
            readonly premultiply: true;
            readonly linearRGB: true;
        };
    };
    readonly rotate: {
        readonly name: "Rotate";
        readonly description: "Rotate image";
        readonly instantiate: () => Promise<(buffer: Uint8Array, width: number, height: number, { numRotations }: RotateOptions) => Promise<ImageData>>;
        readonly defaultOptions: {
            readonly numRotations: 0;
        };
    };
};
export declare const codecs: {
    readonly mozjpeg: {
        readonly name: "MozJPEG";
        readonly extension: "jpg";
        readonly detectors: readonly [RegExp];
        readonly dec: () => Promise<DecodeModule>;
        readonly enc: () => Promise<MozJPEGEncodeModule>;
        readonly defaultEncoderOptions: {
            readonly quality: 75;
            readonly baseline: false;
            readonly arithmetic: false;
            readonly progressive: true;
            readonly optimize_coding: true;
            readonly smoothing: 0;
            readonly color_space: 3;
            readonly quant_table: 3;
            readonly trellis_multipass: false;
            readonly trellis_opt_zero: false;
            readonly trellis_opt_table: false;
            readonly trellis_loops: 1;
            readonly auto_subsample: true;
            readonly chroma_subsample: 2;
            readonly separate_chroma_quality: false;
            readonly chroma_quality: 75;
        };
        readonly autoOptimize: {
            readonly option: "quality";
            readonly min: 0;
            readonly max: 100;
        };
    };
    readonly webp: {
        readonly name: "WebP";
        readonly extension: "webp";
        readonly detectors: readonly [RegExp];
        readonly dec: () => Promise<DecodeModule>;
        readonly enc: () => Promise<WebPEncodeModule>;
        readonly defaultEncoderOptions: {
            readonly quality: 75;
            readonly target_size: 0;
            readonly target_PSNR: 0;
            readonly method: 4;
            readonly sns_strength: 50;
            readonly filter_strength: 60;
            readonly filter_sharpness: 0;
            readonly filter_type: 1;
            readonly partitions: 0;
            readonly segments: 4;
            readonly pass: 1;
            readonly show_compressed: 0;
            readonly preprocessing: 0;
            readonly autofilter: 0;
            readonly partition_limit: 0;
            readonly alpha_compression: 1;
            readonly alpha_filtering: 1;
            readonly alpha_quality: 100;
            readonly lossless: 0;
            readonly exact: 0;
            readonly image_hint: 0;
            readonly emulate_jpeg_size: 0;
            readonly thread_level: 0;
            readonly low_memory: 0;
            readonly near_lossless: 100;
            readonly use_delta_palette: 0;
            readonly use_sharp_yuv: 0;
        };
        readonly autoOptimize: {
            readonly option: "quality";
            readonly min: 0;
            readonly max: 100;
        };
    };
    readonly avif: {
        readonly name: "AVIF";
        readonly extension: "avif";
        readonly detectors: readonly [RegExp];
        readonly dec: () => Promise<DecodeModule>;
        readonly enc: () => Promise<AVIFEncodeModule>;
        readonly defaultEncoderOptions: {
            readonly cqLevel: 33;
            readonly cqAlphaLevel: -1;
            readonly denoiseLevel: 0;
            readonly tileColsLog2: 0;
            readonly tileRowsLog2: 0;
            readonly speed: 6;
            readonly subsample: 1;
            readonly chromaDeltaQ: false;
            readonly sharpness: 0;
            readonly tune: 0;
        };
        readonly autoOptimize: {
            readonly option: "cqLevel";
            readonly min: 62;
            readonly max: 0;
        };
    };
    readonly oxipng: {
        readonly name: "OxiPNG";
        readonly extension: "png";
        readonly detectors: readonly [RegExp];
        readonly dec: () => Promise<{
            decode: (buffer: Buffer | Uint8Array) => any;
        }>;
        readonly enc: () => Promise<{
            encode: (buffer: Uint8ClampedArray | ArrayBuffer, width: number, height: number, opts: {
                level: number;
            }) => any;
        }>;
        readonly defaultEncoderOptions: {
            readonly level: 2;
        };
        readonly autoOptimize: {
            readonly option: "level";
            readonly min: 6;
            readonly max: 1;
        };
    };
};
export {};
