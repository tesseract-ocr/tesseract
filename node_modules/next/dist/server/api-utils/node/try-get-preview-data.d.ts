/// <reference types="node" />
import type { IncomingMessage, ServerResponse } from 'http';
import type { __ApiPreviewProps } from '../.';
import type { BaseNextRequest, BaseNextResponse } from '../../base-http';
import type { PreviewData } from 'next/types';
export declare function tryGetPreviewData(req: IncomingMessage | BaseNextRequest | Request, res: ServerResponse | BaseNextResponse, options: __ApiPreviewProps, multiZoneDraftMode: boolean): PreviewData;
