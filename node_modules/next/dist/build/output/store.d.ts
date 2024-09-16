export type OutputState = {
    bootstrap: true;
    appUrl: string | null;
    bindAddr: string | null;
} | ({
    bootstrap: false;
    appUrl: string | null;
    bindAddr: string | null;
} & ({
    loading: true;
    trigger: string | undefined;
    url: string | undefined;
} | {
    loading: false;
    typeChecking: boolean;
    totalModulesCount: number;
    errors: string[] | null;
    warnings: string[] | null;
    hasEdgeServer: boolean;
}));
export declare function formatTrigger(trigger: string): string;
export declare const store: import("unistore").Store<OutputState>;
