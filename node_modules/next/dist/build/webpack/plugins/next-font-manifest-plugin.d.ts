import { webpack } from 'next/dist/compiled/webpack/webpack';
export type NextFontManifest = {
    pages: {
        [path: string]: string[];
    };
    app: {
        [entry: string]: string[];
    };
    appUsingSizeAdjust: boolean;
    pagesUsingSizeAdjust: boolean;
};
/**
 * The NextFontManifestPlugin collects all font files emitted by next-font-loader and creates a manifest file.
 * The manifest file is used in the Next.js render functions (_document.tsx for pages/ and app-render for app/) to add preload tags for the font files.
 * We only want to att preload fonts that are used by the current route.
 *
 * For pages/ the plugin finds the fonts imported in the entrypoint chunks and creates a map:
 * { [route]: fontFile[] }
 * When rendering the app in _document.tsx, it gets the font files to preload: manifest.pages[currentRouteBeingRendered].
 *
 * For app/, the manifest is a bit different.
 * Instead of creating a map of route to font files, it creates a map of the webpack module request to font files.
 * { [webpackModuleRequest]: fontFile[]]}
 * When creating the component tree in app-render it looks for font files to preload: manifest.app[moduleBeingRendered]
 */
export declare class NextFontManifestPlugin {
    private appDir;
    constructor(options: {
        appDir: undefined | string;
    });
    apply(compiler: webpack.Compiler): void;
}
