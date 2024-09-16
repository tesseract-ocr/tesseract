/// <reference types="node" />
import type { IncomingMessage } from 'http';
import type { SizeLimit } from 'next/types';
/**
 * Parse incoming message like `json` or `urlencoded`
 * @param req request object
 */
export declare function parseBody(req: IncomingMessage, limit: SizeLimit): Promise<any>;
