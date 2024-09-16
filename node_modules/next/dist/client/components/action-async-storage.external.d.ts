/// <reference types="node" />
import type { AsyncLocalStorage } from 'async_hooks';
import { actionAsyncStorage } from './action-async-storage-instance';
export interface ActionStore {
    readonly isAction?: boolean;
    readonly isAppRoute?: boolean;
}
export type ActionAsyncStorage = AsyncLocalStorage<ActionStore>;
export { actionAsyncStorage };
