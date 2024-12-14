import type { IncomingMessage, ServerResponse } from 'http';
import type RenderResult from './render-result';
import type { Revalidate, ExpireTime } from './lib/revalidate';
export declare function sendEtagResponse(req: IncomingMessage, res: ServerResponse, etag: string | undefined): boolean;
export declare function sendRenderResult({ req, res, result, type, generateEtags, poweredByHeader, revalidate, expireTime, }: {
    req: IncomingMessage;
    res: ServerResponse;
    result: RenderResult;
    type: 'html' | 'json' | 'rsc';
    generateEtags: boolean;
    poweredByHeader: boolean;
    revalidate: Revalidate | undefined;
    expireTime: ExpireTime | undefined;
}): Promise<void>;
