export declare const ipcForbiddenHeaders: string[];
export declare const actionsForbiddenHeaders: string[];
export declare const filterReqHeaders: (headers: Record<string, undefined | string | number | string[]>, forbiddenHeaders: string[]) => Record<string, string | string[] | undefined>;
