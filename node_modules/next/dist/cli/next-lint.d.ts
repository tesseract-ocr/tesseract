#!/usr/bin/env node
type NextLintOptions = {
    cache: boolean;
    cacheLocation?: string;
    cacheStrategy: string;
    config?: string;
    dir?: string[];
    errorOnUnmatchedPattern?: boolean;
    ext: string[];
    file?: string[];
    fix?: boolean;
    fixType?: string;
    format?: string;
    ignore: boolean;
    ignorePath?: string;
    inlineConfig: boolean;
    maxWarnings: number;
    outputFile?: string;
    quiet?: boolean;
    reportUnusedDisableDirectivesSeverity: 'error' | 'off' | 'warn';
    resolvePluginsRelativeTo?: string;
    rulesdir?: string;
    strict?: boolean;
};
declare const nextLint: (options: NextLintOptions, directory?: string) => Promise<void>;
export { nextLint };
