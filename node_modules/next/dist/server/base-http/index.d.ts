import type { IncomingHttpHeaders, OutgoingHttpHeaders } from 'http';
import type { I18NConfig } from '../config-shared';
import type { NextApiRequestCookies } from '../api-utils';
export interface BaseNextRequestConfig {
    basePath: string | undefined;
    i18n?: I18NConfig;
    trailingSlash?: boolean | undefined;
}
export type FetchMetric = {
    url: string;
    idx: number;
    end: number;
    start: number;
    method: string;
    status: number;
    cacheReason: string;
    cacheStatus: 'hit' | 'miss' | 'skip' | 'hmr';
    cacheWarning?: string;
};
export type FetchMetrics = Array<FetchMetric>;
export declare abstract class BaseNextRequest<Body = any> {
    method: string;
    url: string;
    body: Body;
    protected _cookies: NextApiRequestCookies | undefined;
    abstract headers: IncomingHttpHeaders;
    abstract fetchMetrics: FetchMetric[] | undefined;
    constructor(method: string, url: string, body: Body);
    get cookies(): Partial<{
        [key: string]: string;
    }>;
}
export declare abstract class BaseNextResponse<Destination = any> {
    destination: Destination;
    abstract statusCode: number | undefined;
    abstract statusMessage: string | undefined;
    abstract get sent(): boolean;
    constructor(destination: Destination);
    /**
     * Sets a value for the header overwriting existing values
     */
    abstract setHeader(name: string, value: string | string[]): this;
    /**
     * Removes a header
     */
    abstract removeHeader(name: string): this;
    /**
     * Appends value for the given header name
     */
    abstract appendHeader(name: string, value: string): this;
    /**
     * Get all vaues for a header as an array or undefined if no value is present
     */
    abstract getHeaderValues(name: string): string[] | undefined;
    abstract hasHeader(name: string): boolean;
    /**
     * Get vaues for a header concatenated using `,` or undefined if no value is present
     */
    abstract getHeader(name: string): string | undefined;
    abstract getHeaders(): OutgoingHttpHeaders;
    abstract body(value: string): this;
    abstract send(): void;
    abstract onClose(callback: () => void): void;
    redirect(destination: string, statusCode: number): this;
}
