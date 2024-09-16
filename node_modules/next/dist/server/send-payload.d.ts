/// <reference types="node" />
import type { IncomingMessage, ServerResponse } from 'http';
import type RenderResult from './render-result';
import type { Revalidate, SwrDelta } from './lib/revalidate';
export declare function sendEtagResponse(req: IncomingMessage, res: ServerResponse, etag: string | undefined): boolean;
export declare function sendRenderResult({ req, res, result, type, generateEtags, poweredByHeader, revalidate, swrDelta, }: {
    req: IncomingMessage;
    res: ServerResponse;
    result: RenderResult;
    type: 'html' | 'json' | 'rsc';
    generateEtags: boolean;
    poweredByHeader: boolean;
    revalidate: Revalidate | undefined;
    swrDelta: SwrDelta | undefined;
}): Promise<void>;
