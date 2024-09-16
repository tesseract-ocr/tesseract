import asyncRetry from 'next/dist/compiled/async-retry';
export declare function retry<T>(fn: asyncRetry.RetryFunction<T>, retries: number): Promise<any>;
