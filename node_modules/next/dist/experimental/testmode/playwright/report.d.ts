import type { TestInfo } from '@playwright/test';
import type { FetchHandler } from './next-worker-fixture';
export declare function reportFetch(testInfo: TestInfo, req: Request, handler: FetchHandler): Promise<Awaited<ReturnType<FetchHandler>>>;
