import type { RenderOpts } from './render';
declare function postProcessHTML(pathname: string, content: string, renderOpts: Pick<RenderOpts, 'ampOptimizerConfig' | 'ampValidator' | 'ampSkipValidation' | 'optimizeFonts' | 'fontManifest' | 'optimizeCss' | 'distDir' | 'assetPrefix'>, { inAmpMode, hybridAmp }: {
    inAmpMode: boolean;
    hybridAmp: boolean;
}): Promise<string>;
export { postProcessHTML };
