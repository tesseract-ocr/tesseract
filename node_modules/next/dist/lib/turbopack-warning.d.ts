import type { NextConfig } from '../server/config-shared';
export declare function validateTurboNextConfig({ dir, isDev, }: {
    dir: string;
    isDev?: boolean;
}): Promise<NextConfig>;
