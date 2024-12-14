#!/usr/bin/env node
export type NextLintOptions = {
    cache: boolean;
    cacheLocation?: string;
    cacheStrategy: string;
    config?: string;
    dir?: string[];
    errorOnUnmatchedPattern?: boolean;
    file?: string[];
    fix?: boolean;
    fixType?: string;
    format?: string;
    ignore: boolean;
    outputFile?: string;
    quiet?: boolean;
    strict?: boolean;
    ext: string[];
    ignorePath?: string;
    reportUnusedDisableDirectivesSeverity: 'error' | 'off' | 'warn';
    resolvePluginsRelativeTo?: string;
    rulesdir?: string;
    inlineConfig: boolean;
    maxWarnings: number;
};
declare const nextLint: (options: NextLintOptions, directory?: string) => Promise<void>;
export { nextLint };
