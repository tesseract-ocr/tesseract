import type { EventLintCheckCompleted } from '../../telemetry/events/build';
declare function lint(baseDir: string, lintDirs: string[], eslintrcFile: string | null, pkgJsonPath: string | null, { lintDuringBuild, eslintOptions, reportErrorsOnly, maxWarnings, formatter, outputFile, }: {
    lintDuringBuild: boolean;
    eslintOptions: any;
    reportErrorsOnly: boolean;
    maxWarnings: number;
    formatter: string | null;
    outputFile: string | null;
}): Promise<string | null | {
    output: string | null;
    isError: boolean;
    eventInfo: EventLintCheckCompleted;
}>;
export declare function runLintCheck(baseDir: string, lintDirs: string[], opts: {
    lintDuringBuild?: boolean;
    eslintOptions?: any;
    reportErrorsOnly?: boolean;
    maxWarnings?: number;
    formatter?: string | null;
    outputFile?: string | null;
    strict?: boolean;
}): ReturnType<typeof lint>;
export {};
