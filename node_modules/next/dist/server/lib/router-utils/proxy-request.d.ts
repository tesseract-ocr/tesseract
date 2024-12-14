import type { IncomingMessage, ServerResponse } from 'http';
import type { NextUrlWithParsedQuery } from '../../request-meta';
import { Duplex } from 'stream';
export declare function proxyRequest(req: IncomingMessage, res: ServerResponse | Duplex, parsedUrl: NextUrlWithParsedQuery, upgradeHead?: Buffer, reqBody?: any, proxyTimeout?: number | null): Promise<boolean>;
