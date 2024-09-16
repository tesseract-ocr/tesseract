import type { webpack } from 'next/dist/compiled/webpack/webpack';
export declare function getClientStyleLoader({ hasAppDir, isAppDir, isDevelopment, assetPrefix, }: {
    hasAppDir: boolean;
    isAppDir?: boolean;
    isDevelopment: boolean;
    assetPrefix: string;
}): webpack.RuleSetUseItem;
