import type { NextApiRequestCookies } from '.';
/**
 * Parse cookies from the `headers` of request
 * @param req request object
 */
export declare function getCookieParser(headers: {
    [key: string]: string | string[] | null | undefined;
}): () => NextApiRequestCookies;
