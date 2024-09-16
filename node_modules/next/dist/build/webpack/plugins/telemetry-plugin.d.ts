import type { webpack } from 'next/dist/compiled/webpack/webpack';
/**
 * List of target triples next-swc native binary supports.
 */
export type SWC_TARGET_TRIPLE = 'x86_64-apple-darwin' | 'x86_64-unknown-linux-gnu' | 'x86_64-pc-windows-msvc' | 'i686-pc-windows-msvc' | 'aarch64-unknown-linux-gnu' | 'armv7-unknown-linux-gnueabihf' | 'aarch64-apple-darwin' | 'aarch64-linux-android' | 'arm-linux-androideabi' | 'x86_64-unknown-freebsd' | 'x86_64-unknown-linux-musl' | 'aarch64-unknown-linux-musl' | 'aarch64-pc-windows-msvc';
export type Feature = 'next/image' | 'next/future/image' | 'next/legacy/image' | 'next/script' | 'next/dynamic' | '@next/font/google' | '@next/font/local' | 'next/font/google' | 'next/font/local' | 'swcLoader' | 'swcMinify' | 'swcRelay' | 'swcStyledComponents' | 'swcReactRemoveProperties' | 'swcExperimentalDecorators' | 'swcRemoveConsole' | 'swcImportSource' | 'swcEmotion' | `swc/target/${SWC_TARGET_TRIPLE}` | 'turbotrace' | 'transpilePackages' | 'skipMiddlewareUrlNormalize' | 'skipTrailingSlashRedirect' | 'modularizeImports';
interface FeatureUsage {
    featureName: Feature;
    invocationCount: number;
}
/**
 * Plugin that queries the ModuleGraph to look for modules that correspond to
 * certain features (e.g. next/image and next/script) and record how many times
 * they are imported.
 */
export declare class TelemetryPlugin implements webpack.WebpackPluginInstance {
    private usageTracker;
    constructor(buildFeaturesMap: Map<Feature, boolean>);
    apply(compiler: webpack.Compiler): void;
    usages(): FeatureUsage[];
    packagesUsedInServerSideProps(): string[];
}
export type TelemetryPluginState = {
    usages: ReturnType<TelemetryPlugin['usages']>;
    packagesUsedInServerSideProps: ReturnType<TelemetryPlugin['packagesUsedInServerSideProps']>;
};
export {};
