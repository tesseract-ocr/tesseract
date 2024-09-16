import type { ProxyFetchRequest, ProxyResponse } from './types';
export type FetchHandlerResult = Response | 'abort' | 'continue' | null | undefined;
export type FetchHandler = (testData: string, request: Request) => FetchHandlerResult | Promise<FetchHandlerResult>;
export declare function handleFetch(req: ProxyFetchRequest, onFetch: FetchHandler): Promise<ProxyResponse>;
