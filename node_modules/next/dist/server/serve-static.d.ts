/// <reference types="node" />
/// <reference types="send" />
import type { IncomingMessage, ServerResponse } from 'http';
import send from 'next/dist/compiled/send';
export declare function serveStatic(req: IncomingMessage, res: ServerResponse, path: string, opts?: Parameters<typeof send>[2]): Promise<void>;
export declare const getContentType: (extWithoutDot: string) => string | null;
export declare const getExtension: (contentType: string) => string | null;
