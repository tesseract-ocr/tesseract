import Router from '../shared/lib/router/router';
import type { NextRouter } from '../shared/lib/router/router';
type SingletonRouterBase = {
    router: Router | null;
    readyCallbacks: Array<() => any>;
    ready(cb: () => any): void;
};
export { Router };
export type { NextRouter };
export type SingletonRouter = SingletonRouterBase & NextRouter;
declare const routerEvents: readonly ["routeChangeStart", "beforeHistoryChange", "routeChangeComplete", "routeChangeError", "hashChangeStart", "hashChangeComplete"];
export type RouterEvent = (typeof routerEvents)[number];
declare const _default: SingletonRouter;
export default _default;
export { default as withRouter } from './with-router';
/**
 * This hook gives access the [router object](https://nextjs.org/docs/pages/api-reference/functions/use-router#router-object)
 * inside the [Pages Router](https://nextjs.org/docs/pages/building-your-application).
 *
 * Read more: [Next.js Docs: `useRouter`](https://nextjs.org/docs/pages/api-reference/functions/use-router)
 */
export declare function useRouter(): NextRouter;
