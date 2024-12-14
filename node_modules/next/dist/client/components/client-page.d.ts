import type { ParsedUrlQuery } from 'querystring';
import type { Params } from '../../server/request/params';
/**
 * When the Page is a client component we send the params and searchParams to this client wrapper
 * where they are turned into dynamically tracked values before being passed to the actual Page component.
 *
 * additionally we may send promises representing the params and searchParams. We don't ever use these passed
 * values but it can be necessary for the sender to send a Promise that doesn't resolve in certain situations.
 * It is up to the caller to decide if the promises are needed.
 */
export declare function ClientPageRoot({ Component, searchParams, params, promises, }: {
    Component: React.ComponentType<any>;
    searchParams: ParsedUrlQuery;
    params: Params;
    promises?: Array<Promise<any>>;
}): import("react/jsx-runtime").JSX.Element;
