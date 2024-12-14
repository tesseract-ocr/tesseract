declare class ReadonlyURLSearchParams extends URLSearchParams {
    /** @deprecated Method unavailable on `ReadonlyURLSearchParams`. Read more: https://nextjs.org/docs/app/api-reference/functions/use-search-params#updating-searchparams */
    append(): void;
    /** @deprecated Method unavailable on `ReadonlyURLSearchParams`. Read more: https://nextjs.org/docs/app/api-reference/functions/use-search-params#updating-searchparams */
    delete(): void;
    /** @deprecated Method unavailable on `ReadonlyURLSearchParams`. Read more: https://nextjs.org/docs/app/api-reference/functions/use-search-params#updating-searchparams */
    set(): void;
    /** @deprecated Method unavailable on `ReadonlyURLSearchParams`. Read more: https://nextjs.org/docs/app/api-reference/functions/use-search-params#updating-searchparams */
    sort(): void;
}
export { redirect, permanentRedirect } from './redirect';
export { RedirectType } from './redirect-error';
export { notFound } from './not-found';
export { forbidden } from './forbidden';
export { unauthorized } from './unauthorized';
export { unstable_rethrow } from './unstable-rethrow';
export { ReadonlyURLSearchParams };
