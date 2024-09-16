/// <reference types="node" />
import type { AsyncLocalStorage } from 'async_hooks';
import type { DraftModeProvider } from '../../server/async-storage/draft-mode-provider';
import type { ResponseCookies } from '../../server/web/spec-extension/cookies';
import type { ReadonlyHeaders } from '../../server/web/spec-extension/adapters/headers';
import type { ReadonlyRequestCookies } from '../../server/web/spec-extension/adapters/request-cookies';
import { requestAsyncStorage } from './request-async-storage-instance';
import type { DeepReadonly } from '../../shared/lib/deep-readonly';
export interface RequestStore {
    readonly headers: ReadonlyHeaders;
    readonly cookies: ReadonlyRequestCookies;
    readonly mutableCookies: ResponseCookies;
    readonly draftMode: DraftModeProvider;
    readonly reactLoadableManifest: DeepReadonly<Record<string, {
        files: string[];
    }>>;
    readonly assetPrefix: string;
}
export type RequestAsyncStorage = AsyncLocalStorage<RequestStore>;
export { requestAsyncStorage };
export declare function getExpectedRequestStore(callingExpression: string): RequestStore;
