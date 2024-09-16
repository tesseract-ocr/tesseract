/// <reference types="node" />
import type { IncomingMessage } from 'http';
import type { ReadonlyRequestCookies } from '../web/spec-extension/adapters/request-cookies';
import type { ResponseCookies } from '../web/spec-extension/cookies';
import type { BaseNextRequest } from '../base-http';
import type { NextRequest } from '../web/spec-extension/request';
import type { __ApiPreviewProps } from '../api-utils';
export declare class DraftModeProvider {
    readonly isEnabled: boolean;
    constructor(previewProps: __ApiPreviewProps | undefined, req: IncomingMessage | BaseNextRequest<unknown> | NextRequest, cookies: ReadonlyRequestCookies, mutableCookies: ResponseCookies);
    enable(): void;
    disable(): void;
}
