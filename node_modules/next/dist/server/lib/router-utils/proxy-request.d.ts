/// <reference types="node" />
import type { IncomingMessage, ServerResponse } from 'http';
import type { NextUrlWithParsedQuery } from '../../request-meta';
export declare function proxyRequest(req: IncomingMessage, res: ServerResponse, parsedUrl: NextUrlWithParsedQuery, upgradeHead?: any, reqBody?: any, proxyTimeout?: number | null): Promise<void>;
