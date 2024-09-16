import { webpack } from 'next/dist/compiled/webpack/webpack';
import type { FontManifest } from '../../../server/font-utils';
export declare class FontStylesheetGatheringPlugin {
    compiler?: webpack.Compiler;
    gatheredStylesheets: Array<string>;
    manifestContent: FontManifest;
    adjustFontFallbacks?: boolean;
    adjustFontFallbacksWithSizeAdjust?: boolean;
    constructor({ adjustFontFallbacks, adjustFontFallbacksWithSizeAdjust, }: {
        adjustFontFallbacks?: boolean;
        adjustFontFallbacksWithSizeAdjust?: boolean;
    });
    private parserHandler;
    apply(compiler: webpack.Compiler): void;
}
