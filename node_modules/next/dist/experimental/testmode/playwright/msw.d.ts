import { defineConfig } from './index';
import type { NextFixture } from './next-fixture';
import { type RequestHandler } from 'msw';
export * from 'msw';
export * from '@playwright/test';
export type { NextFixture };
export { defineConfig };
export interface MswFixture {
    use: (...handlers: RequestHandler[]) => void;
}
export declare const test: import("@playwright/test").TestType<import("@playwright/test").PlaywrightTestArgs & import("@playwright/test").PlaywrightTestOptions & {
    next: NextFixture;
    nextOptions: import("./next-options").NextOptions;
} & {
    msw: MswFixture;
    mswHandlers: RequestHandler[];
}, import("@playwright/test").PlaywrightWorkerArgs & import("@playwright/test").PlaywrightWorkerOptions & {
    _nextWorker: import("./next-worker-fixture").NextWorkerFixture;
}>;
export default test;
