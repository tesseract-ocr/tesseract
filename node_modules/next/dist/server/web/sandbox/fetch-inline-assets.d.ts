import type { EdgeFunctionDefinition } from '../../../build/webpack/plugins/middleware-plugin';
/**
 * Short-circuits the `fetch` function
 * to return a stream for a given asset, if a user used `new URL("file", import.meta.url)`.
 * This allows to embed assets in Edge Runtime.
 */
export declare function fetchInlineAsset(options: {
    input: RequestInfo | URL;
    distDir: string;
    assets: EdgeFunctionDefinition['assets'];
    context: {
        Response: typeof Response;
        ReadableStream: typeof ReadableStream;
    };
}): Promise<Response | undefined>;
