/// <reference types="node" />
import type { ParsedUrlQuery } from 'querystring';
/**
 * Takes a ParsedUrlQuery object and either returns it unmodified or returns an empty object
 *
 * Even though we do not track read access on the returned searchParams we need to
 * return an empty object if we are doing a 'force-static' render. This is to ensure
 * we don't encode the searchParams into the flight data.
 */
export declare function createUntrackedSearchParams(searchParams: ParsedUrlQuery): ParsedUrlQuery;
/**
 * Takes a ParsedUrlQuery object and returns a Proxy that tracks read access to the object
 *
 * If running in the browser will always return the provided searchParams object.
 * When running during SSR will return empty during a 'force-static' render and
 * otherwise it returns a searchParams object which tracks reads to trigger dynamic rendering
 * behavior if appropriate
 */
export declare function createDynamicallyTrackedSearchParams(searchParams: ParsedUrlQuery): ParsedUrlQuery;
