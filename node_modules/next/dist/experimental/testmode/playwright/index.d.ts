import * as base from '@playwright/test';
import type { NextFixture } from './next-fixture';
import type { NextOptions } from './next-options';
import type { NextWorkerFixture } from './next-worker-fixture';
export * from '@playwright/test';
export type { NextFixture, NextOptions };
export type { FetchHandlerResult } from '../proxy';
export interface NextOptionsConfig {
    nextOptions?: NextOptions;
}
export declare function defineConfig<T extends NextOptionsConfig, W>(config: base.PlaywrightTestConfig<T, W>): base.PlaywrightTestConfig<T, W>;
export declare const test: base.TestType<base.PlaywrightTestArgs & base.PlaywrightTestOptions & {
    next: NextFixture;
    nextOptions: NextOptions;
}, base.PlaywrightWorkerArgs & base.PlaywrightWorkerOptions & {
    _nextWorker: NextWorkerFixture;
}>;
export default test;
