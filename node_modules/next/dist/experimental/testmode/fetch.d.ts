import { type TestRequestReader } from './context';
type Fetch = typeof fetch;
export declare const reader: TestRequestReader<Request>;
export declare function handleFetch(originalFetch: Fetch, request: Request): Promise<Response>;
export declare function interceptFetch(originalFetch: Fetch): () => void;
export {};
