import type { NextConfigComplete } from '../server/config-shared';
import type { Telemetry } from '../telemetry/storage';
import type { Span } from '../trace';
export declare function startTypeChecking({ cacheDir, config, dir, ignoreESLint, nextBuildSpan, pagesDir, runLint, shouldLint, telemetry, appDir, }: {
    cacheDir: string;
    config: NextConfigComplete;
    dir: string;
    ignoreESLint: boolean;
    nextBuildSpan: Span;
    pagesDir?: string;
    runLint: boolean;
    shouldLint: boolean;
    telemetry: Telemetry;
    appDir?: string;
}): Promise<void>;
