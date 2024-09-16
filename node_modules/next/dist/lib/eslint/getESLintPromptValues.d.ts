export declare const getESLintStrictValue: (cwd: string) => Promise<{
    title: string;
    recommended: boolean;
    config: {
        extends: string | string[];
    };
}>;
export declare const getESLintPromptValues: (cwd: string) => Promise<({
    title: string;
    recommended: boolean;
    config: {
        extends: string | string[];
    };
} | {
    title: string;
    config: {
        extends: string;
    };
} | {
    title: string;
    config: null;
})[]>;
