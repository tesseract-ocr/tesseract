/**
 * This function allows you to purge [cached data](https://nextjs.org/docs/app/building-your-application/caching) on-demand for a specific cache tag.
 *
 * Read more: [Next.js Docs: `revalidateTag`](https://nextjs.org/docs/app/api-reference/functions/revalidateTag)
 */
export declare function revalidateTag(tag: string): void;
/**
 * This function allows you to purge [cached data](https://nextjs.org/docs/app/building-your-application/caching) on-demand for a specific path.
 *
 * Read more: [Next.js Docs: `unstable_expirePath`](https://nextjs.org/docs/app/api-reference/functions/unstable_expirePath)
 */
export declare function unstable_expirePath(originalPath: string, type?: 'layout' | 'page'): void;
/**
 * This function allows you to purge [cached data](https://nextjs.org/docs/app/building-your-application/caching) on-demand for a specific cache tag.
 *
 * Read more: [Next.js Docs: `unstable_expireTag`](https://nextjs.org/docs/app/api-reference/functions/unstable_expireTag)
 */
export declare function unstable_expireTag(...tags: string[]): void;
/**
 * This function allows you to purge [cached data](https://nextjs.org/docs/app/building-your-application/caching) on-demand for a specific path.
 *
 * Read more: [Next.js Docs: `revalidatePath`](https://nextjs.org/docs/app/api-reference/functions/revalidatePath)
 */
export declare function revalidatePath(originalPath: string, type?: 'layout' | 'page'): void;
