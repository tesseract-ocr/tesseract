export declare enum MessageSeverity {
    Warning = 1,
    Error = 2
}
interface LintMessage {
    ruleId: string | null;
    severity: 1 | 2;
    message: string;
    line: number;
    column: number;
}
export interface LintResult {
    filePath: string;
    messages: LintMessage[];
    errorCount: number;
    warningCount: number;
    output?: string;
    source?: string;
}
export declare function formatResults(baseDir: string, results: LintResult[], format: (r: LintResult[]) => string | Promise<string>): Promise<{
    output: string;
    outputWithMessages: string;
    totalNextPluginErrorCount: number;
    totalNextPluginWarningCount: number;
}>;
export {};
