/// <reference types="node" />
import type { AsyncLocalStorage } from 'async_hooks';
export declare function createAsyncLocalStorage<Store extends {}>(): AsyncLocalStorage<Store>;
