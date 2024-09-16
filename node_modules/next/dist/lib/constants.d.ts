import type { ServerRuntime } from '../../types';
export declare const NEXT_QUERY_PARAM_PREFIX = "nxtP";
export declare const PRERENDER_REVALIDATE_HEADER = "x-prerender-revalidate";
export declare const PRERENDER_REVALIDATE_ONLY_GENERATED_HEADER = "x-prerender-revalidate-if-generated";
export declare const RSC_PREFETCH_SUFFIX = ".prefetch.rsc";
export declare const RSC_SUFFIX = ".rsc";
export declare const ACTION_SUFFIX = ".action";
export declare const NEXT_DATA_SUFFIX = ".json";
export declare const NEXT_META_SUFFIX = ".meta";
export declare const NEXT_BODY_SUFFIX = ".body";
export declare const NEXT_CACHE_TAGS_HEADER = "x-next-cache-tags";
export declare const NEXT_CACHE_SOFT_TAGS_HEADER = "x-next-cache-soft-tags";
export declare const NEXT_CACHE_REVALIDATED_TAGS_HEADER = "x-next-revalidated-tags";
export declare const NEXT_CACHE_REVALIDATE_TAG_TOKEN_HEADER = "x-next-revalidate-tag-token";
export declare const NEXT_CACHE_TAG_MAX_ITEMS = 64;
export declare const NEXT_CACHE_TAG_MAX_LENGTH = 256;
export declare const NEXT_CACHE_SOFT_TAG_MAX_LENGTH = 1024;
export declare const NEXT_CACHE_IMPLICIT_TAG_ID = "_N_T_";
export declare const CACHE_ONE_YEAR = 31536000;
export declare const MIDDLEWARE_FILENAME = "middleware";
export declare const MIDDLEWARE_LOCATION_REGEXP = "(?:src/)?middleware";
export declare const INSTRUMENTATION_HOOK_FILENAME = "instrumentation";
export declare const PAGES_DIR_ALIAS = "private-next-pages";
export declare const DOT_NEXT_ALIAS = "private-dot-next";
export declare const ROOT_DIR_ALIAS = "private-next-root-dir";
export declare const APP_DIR_ALIAS = "private-next-app-dir";
export declare const RSC_MOD_REF_PROXY_ALIAS = "private-next-rsc-mod-ref-proxy";
export declare const RSC_ACTION_VALIDATE_ALIAS = "private-next-rsc-action-validate";
export declare const RSC_ACTION_PROXY_ALIAS = "private-next-rsc-server-reference";
export declare const RSC_ACTION_ENCRYPTION_ALIAS = "private-next-rsc-action-encryption";
export declare const RSC_ACTION_CLIENT_WRAPPER_ALIAS = "private-next-rsc-action-client-wrapper";
export declare const PUBLIC_DIR_MIDDLEWARE_CONFLICT = "You can not have a '_next' folder inside of your public folder. This conflicts with the internal '/_next' route. https://nextjs.org/docs/messages/public-next-folder-conflict";
export declare const SSG_GET_INITIAL_PROPS_CONFLICT = "You can not use getInitialProps with getStaticProps. To use SSG, please remove your getInitialProps";
export declare const SERVER_PROPS_GET_INIT_PROPS_CONFLICT = "You can not use getInitialProps with getServerSideProps. Please remove getInitialProps.";
export declare const SERVER_PROPS_SSG_CONFLICT = "You can not use getStaticProps or getStaticPaths with getServerSideProps. To use SSG, please remove getServerSideProps";
export declare const STATIC_STATUS_PAGE_GET_INITIAL_PROPS_ERROR = "can not have getInitialProps/getServerSideProps, https://nextjs.org/docs/messages/404-get-initial-props";
export declare const SERVER_PROPS_EXPORT_ERROR = "pages with `getServerSideProps` can not be exported. See more info here: https://nextjs.org/docs/messages/gssp-export";
export declare const GSP_NO_RETURNED_VALUE = "Your `getStaticProps` function did not return an object. Did you forget to add a `return`?";
export declare const GSSP_NO_RETURNED_VALUE = "Your `getServerSideProps` function did not return an object. Did you forget to add a `return`?";
export declare const UNSTABLE_REVALIDATE_RENAME_ERROR: string;
export declare const GSSP_COMPONENT_MEMBER_ERROR = "can not be attached to a page's component and must be exported from the page. See more info here: https://nextjs.org/docs/messages/gssp-component-member";
export declare const NON_STANDARD_NODE_ENV = "You are using a non-standard \"NODE_ENV\" value in your environment. This creates inconsistencies in the project and is strongly advised against. Read more: https://nextjs.org/docs/messages/non-standard-node-env";
export declare const SSG_FALLBACK_EXPORT_ERROR = "Pages with `fallback` enabled in `getStaticPaths` can not be exported. See more info here: https://nextjs.org/docs/messages/ssg-fallback-true-export";
export declare const ESLINT_DEFAULT_DIRS: string[];
export declare const SERVER_RUNTIME: Record<string, ServerRuntime>;
/**
 * The names of the webpack layers. These layers are the primitives for the
 * webpack chunks.
 */
declare const WEBPACK_LAYERS_NAMES: {
    /**
     * The layer for the shared code between the client and server bundles.
     */
    readonly shared: "shared";
    /**
     * React Server Components layer (rsc).
     */
    readonly reactServerComponents: "rsc";
    /**
     * Server Side Rendering layer for app (ssr).
     */
    readonly serverSideRendering: "ssr";
    /**
     * The browser client bundle layer for actions.
     */
    readonly actionBrowser: "action-browser";
    /**
     * The layer for the API routes.
     */
    readonly api: "api";
    /**
     * The layer for the middleware code.
     */
    readonly middleware: "middleware";
    /**
     * The layer for the instrumentation hooks.
     */
    readonly instrument: "instrument";
    /**
     * The layer for assets on the edge.
     */
    readonly edgeAsset: "edge-asset";
    /**
     * The browser client bundle layer for App directory.
     */
    readonly appPagesBrowser: "app-pages-browser";
    /**
     * The server bundle layer for metadata routes.
     */
    readonly appMetadataRoute: "app-metadata-route";
    /**
     * The layer for the server bundle for App Route handlers.
     */
    readonly appRouteHandler: "app-route-handler";
};
export type WebpackLayerName = (typeof WEBPACK_LAYERS_NAMES)[keyof typeof WEBPACK_LAYERS_NAMES];
declare const WEBPACK_LAYERS: {
    GROUP: {
        serverOnly: ("rsc" | "action-browser" | "instrument" | "app-metadata-route" | "app-route-handler")[];
        clientOnly: ("ssr" | "app-pages-browser")[];
        nonClientServerTarget: ("middleware" | "api")[];
        app: ("shared" | "rsc" | "ssr" | "action-browser" | "instrument" | "app-pages-browser" | "app-metadata-route" | "app-route-handler")[];
    };
    shared: "shared";
    reactServerComponents: "rsc";
    serverSideRendering: "ssr";
    actionBrowser: "action-browser";
    api: "api";
    middleware: "middleware";
    instrument: "instrument";
    edgeAsset: "edge-asset";
    appPagesBrowser: "app-pages-browser";
    appMetadataRoute: "app-metadata-route";
    appRouteHandler: "app-route-handler";
};
declare const WEBPACK_RESOURCE_QUERIES: {
    edgeSSREntry: string;
    metadata: string;
    metadataRoute: string;
    metadataImageMeta: string;
};
export { WEBPACK_LAYERS, WEBPACK_RESOURCE_QUERIES };
