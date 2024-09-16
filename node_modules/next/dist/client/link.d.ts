/// <reference types="node" />
import React from 'react';
import type { UrlObject } from 'url';
type Url = string | UrlObject;
type InternalLinkProps = {
    /**
     * The path or URL to navigate to. It can also be an object.
     *
     * @example https://nextjs.org/docs/api-reference/next/link#with-url-object
     */
    href: Url;
    /**
     * Optional decorator for the path that will be shown in the browser URL bar. Before Next.js 9.5.3 this was used for dynamic routes, check our [previous docs](https://github.com/vercel/next.js/blob/v9.5.2/docs/api-reference/next/link.md#dynamic-routes) to see how it worked. Note: when this path differs from the one provided in `href` the previous `href`/`as` behavior is used as shown in the [previous docs](https://github.com/vercel/next.js/blob/v9.5.2/docs/api-reference/next/link.md#dynamic-routes).
     */
    as?: Url;
    /**
     * Replace the current `history` state instead of adding a new url into the stack.
     *
     * @defaultValue `false`
     */
    replace?: boolean;
    /**
     * Whether to override the default scroll behavior
     *
     * @example https://nextjs.org/docs/api-reference/next/link#disable-scrolling-to-the-top-of-the-page
     *
     * @defaultValue `true`
     */
    scroll?: boolean;
    /**
     * Update the path of the current page without rerunning [`getStaticProps`](/docs/pages/building-your-application/data-fetching/get-static-props), [`getServerSideProps`](/docs/pages/building-your-application/data-fetching/get-server-side-props) or [`getInitialProps`](/docs/pages/api-reference/functions/get-initial-props).
     *
     * @defaultValue `false`
     */
    shallow?: boolean;
    /**
     * Forces `Link` to send the `href` property to its child.
     *
     * @defaultValue `false`
     */
    passHref?: boolean;
    /**
     * Prefetch the page in the background.
     * Any `<Link />` that is in the viewport (initially or through scroll) will be preloaded.
     * Prefetch can be disabled by passing `prefetch={false}`. When `prefetch` is set to `false`, prefetching will still occur on hover in pages router but not in app router. Pages using [Static Generation](/docs/basic-features/data-fetching/get-static-props.md) will preload `JSON` files with the data for faster page transitions. Prefetching is only enabled in production.
     *
     * @defaultValue `true`
     */
    prefetch?: boolean;
    /**
     * The active locale is automatically prepended. `locale` allows for providing a different locale.
     * When `false` `href` has to include the locale as the default behavior is disabled.
     */
    locale?: string | false;
    /**
     * Enable legacy link behavior.
     * @defaultValue `false`
     * @see https://github.com/vercel/next.js/commit/489e65ed98544e69b0afd7e0cfc3f9f6c2b803b7
     */
    legacyBehavior?: boolean;
    /**
     * Optional event handler for when the mouse pointer is moved onto Link
     */
    onMouseEnter?: React.MouseEventHandler<HTMLAnchorElement>;
    /**
     * Optional event handler for when Link is touched.
     */
    onTouchStart?: React.TouchEventHandler<HTMLAnchorElement>;
    /**
     * Optional event handler for when Link is clicked.
     */
    onClick?: React.MouseEventHandler<HTMLAnchorElement>;
};
export type LinkProps<RouteInferType = any> = InternalLinkProps;
/**
 * A React component that extends the HTML `<a>` element to provide [prefetching](https://nextjs.org/docs/app/building-your-application/routing/linking-and-navigating#2-prefetching)
 * and client-side navigation between routes.
 *
 * It is the primary way to navigate between routes in Next.js.
 *
 * Read more: [Next.js docs: `<Link>`](https://nextjs.org/docs/app/api-reference/components/link)
 */
declare const Link: React.ForwardRefExoticComponent<Omit<React.AnchorHTMLAttributes<HTMLAnchorElement>, keyof InternalLinkProps> & InternalLinkProps & {
    children?: React.ReactNode;
} & React.RefAttributes<HTMLAnchorElement>>;
export default Link;
