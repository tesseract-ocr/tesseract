/// <reference types="node" />
import type { IncomingMessage, ServerResponse } from 'http';
import type { __ApiPreviewProps } from '../.';
type RevalidateFn = (config: {
    urlPath: string;
    revalidateHeaders: {
        [key: string]: string | string[];
    };
    opts: {
        unstable_onlyGenerated?: boolean;
    };
}) => Promise<void>;
type ApiContext = __ApiPreviewProps & {
    trustHostHeader?: boolean;
    allowedRevalidateHeaderKeys?: string[];
    hostname?: string;
    revalidate?: RevalidateFn;
    multiZoneDraftMode?: boolean;
};
export declare function apiResolver(req: IncomingMessage, res: ServerResponse, query: any, resolverModule: any, apiContext: ApiContext, propagateError: boolean, dev?: boolean, page?: string): Promise<void>;
export {};
