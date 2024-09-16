import type { webpack } from 'next/dist/compiled/webpack/webpack';
export declare function startedDevelopmentServer(appUrl: string, bindAddr: string): void;
type AmpStatus = {
    message: string;
    line: number;
    col: number;
    specUrl: string | null;
    code: string;
};
export type AmpPageStatus = {
    [page: string]: {
        errors: AmpStatus[];
        warnings: AmpStatus[];
    };
};
export declare function formatAmpMessages(amp: AmpPageStatus): string;
export declare function ampValidation(page: string, errors: AmpStatus[], warnings: AmpStatus[]): void;
export declare function watchCompilers(client: webpack.Compiler, server: webpack.Compiler, edgeServer: webpack.Compiler): void;
export declare function reportTrigger(trigger: string, url?: string): void;
export {};
