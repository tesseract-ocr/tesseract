import type { FetchHandlerResult } from '../proxy';
export type FetchHandler = (request: Request) => FetchHandlerResult | Promise<FetchHandlerResult>;
export interface NextWorkerFixture {
    proxyPort: number;
    onFetch: (testId: string, handler: FetchHandler) => void;
    cleanupTest: (testId: string) => void;
}
export declare function applyNextWorkerFixture(use: (fixture: NextWorkerFixture) => Promise<void>): Promise<void>;
