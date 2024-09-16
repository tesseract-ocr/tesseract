"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "NextTypesPlugin", {
    enumerable: true,
    get: function() {
        return NextTypesPlugin;
    }
});
const _promises = /*#__PURE__*/ _interop_require_default(require("fs/promises"));
const _webpack = require("next/dist/compiled/webpack/webpack");
const _pathtoregexp = require("next/dist/compiled/path-to-regexp");
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _constants = require("../../../../lib/constants");
const _denormalizepagepath = require("../../../../shared/lib/page-path/denormalize-page-path");
const _ensureleadingslash = require("../../../../shared/lib/page-path/ensure-leading-slash");
const _normalizepathsep = require("../../../../shared/lib/page-path/normalize-path-sep");
const _http = require("../../../../server/web/http");
const _utils = require("../../../../shared/lib/router/utils");
const _apppaths = require("../../../../shared/lib/router/utils/app-paths");
const _entries = require("../../../entries");
const _shared = require("./shared");
const _buildcontext = require("../../../build-context");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const PLUGIN_NAME = "NextTypesPlugin";
function createTypeGuardFile(fullPath, relativePath, options) {
    return `// File: ${fullPath}
import * as entry from '${relativePath}.js'
${options.type === "route" ? `import type { NextRequest } from 'next/server.js'` : `import type { ResolvingMetadata, ResolvingViewport } from 'next/dist/lib/metadata/types/metadata-interface.js'`}

type TEntry = typeof import('${relativePath}.js')

// Check that the entry is a valid entry
checkFields<Diff<{
  ${options.type === "route" ? _http.HTTP_METHODS.map((method)=>`${method}?: Function`).join("\n  ") : "default: Function"}
  config?: {}
  generateStaticParams?: Function
  revalidate?: RevalidateRange<TEntry> | false
  dynamic?: 'auto' | 'force-dynamic' | 'error' | 'force-static'
  dynamicParams?: boolean
  fetchCache?: 'auto' | 'force-no-store' | 'only-no-store' | 'default-no-store' | 'default-cache' | 'only-cache' | 'force-cache'
  preferredRegion?: 'auto' | 'global' | 'home' | string | string[]
  runtime?: 'nodejs' | 'experimental-edge' | 'edge'
  maxDuration?: number
  ${options.type === "route" ? "" : `
  metadata?: any
  generateMetadata?: Function
  viewport?: any
  generateViewport?: Function
  `}
}, TEntry, ''>>()

${options.type === "route" ? _http.HTTP_METHODS.map((method)=>`// Check the prop type of the entry function
if ('${method}' in entry) {
  checkFields<
    Diff<
      ParamCheck<Request | NextRequest>,
      {
        __tag__: '${method}'
        __param_position__: 'first'
        __param_type__: FirstArg<MaybeField<TEntry, '${method}'>>
      },
      '${method}'
    >
  >()
  checkFields<
    Diff<
      ParamCheck<PageParams>,
      {
        __tag__: '${method}'
        __param_position__: 'second'
        __param_type__: SecondArg<MaybeField<TEntry, '${method}'>>
      },
      '${method}'
    >
  >()
  ${""}
  checkFields<
    Diff<
      {
        __tag__: '${method}',
        __return_type__: Response | void | never | Promise<Response | void | never>
      },
      {
        __tag__: '${method}',
        __return_type__: ReturnType<MaybeField<TEntry, '${method}'>>
      },
      '${method}'
    >
  >()
}
`).join("") : `// Check the prop type of the entry function
checkFields<Diff<${options.type === "page" ? "PageProps" : "LayoutProps"}, FirstArg<TEntry['default']>, 'default'>>()

// Check the arguments and return type of the generateMetadata function
if ('generateMetadata' in entry) {
  checkFields<Diff<${options.type === "page" ? "PageProps" : "LayoutProps"}, FirstArg<MaybeField<TEntry, 'generateMetadata'>>, 'generateMetadata'>>()
  checkFields<Diff<ResolvingMetadata, SecondArg<MaybeField<TEntry, 'generateMetadata'>>, 'generateMetadata'>>()
}

// Check the arguments and return type of the generateViewport function
if ('generateViewport' in entry) {
  checkFields<Diff<${options.type === "page" ? "PageProps" : "LayoutProps"}, FirstArg<MaybeField<TEntry, 'generateViewport'>>, 'generateViewport'>>()
  checkFields<Diff<ResolvingViewport, SecondArg<MaybeField<TEntry, 'generateViewport'>>, 'generateViewport'>>()
}
`}
// Check the arguments and return type of the generateStaticParams function
if ('generateStaticParams' in entry) {
  checkFields<Diff<{ params: PageParams }, FirstArg<MaybeField<TEntry, 'generateStaticParams'>>, 'generateStaticParams'>>()
  checkFields<Diff<{ __tag__: 'generateStaticParams', __return_type__: any[] | Promise<any[]> }, { __tag__: 'generateStaticParams', __return_type__: ReturnType<MaybeField<TEntry, 'generateStaticParams'>> }>>()
}

type PageParams = any
export interface PageProps {
  params?: any
  searchParams?: any
}
export interface LayoutProps {
  children?: React.ReactNode
${options.slots ? options.slots.map((slot)=>`  ${slot}: React.ReactNode`).join("\n") : ""}
  params?: any
}

// =============
// Utility types
type RevalidateRange<T> = T extends { revalidate: any } ? NonNegative<T['revalidate']> : never

// If T is unknown or any, it will be an empty {} type. Otherwise, it will be the same as Omit<T, keyof Base>.
type OmitWithTag<T, K extends keyof any, _M> = Omit<T, K>
type Diff<Base, T extends Base, Message extends string = ''> = 0 extends (1 & T) ? {} : OmitWithTag<T, keyof Base, Message>

type FirstArg<T extends Function> = T extends (...args: [infer T, any]) => any ? unknown extends T ? any : T : never
type SecondArg<T extends Function> = T extends (...args: [any, infer T]) => any ? unknown extends T ? any : T : never
type MaybeField<T, K extends string> = T extends { [k in K]: infer G } ? G extends Function ? G : never : never

${options.type === "route" ? `type ParamCheck<T> = {
  __tag__: string
  __param_position__: string
  __param_type__: T
}` : ""}

function checkFields<_ extends { [k in keyof any]: never }>() {}

// https://github.com/sindresorhus/type-fest
type Numeric = number | bigint
type Zero = 0 | 0n
type Negative<T extends Numeric> = T extends Zero ? never : \`\${T}\` extends \`-\${string}\` ? T : never
type NonNegative<T extends Numeric> = T extends Zero ? T : Negative<T> extends never ? T : '__invalid_negative_number__'
`;
}
async function collectNamedSlots(layoutPath) {
    const layoutDir = _path.default.dirname(layoutPath);
    const items = await _promises.default.readdir(layoutDir, {
        withFileTypes: true
    });
    const slots = [];
    for (const item of items){
        if (item.isDirectory() && item.name.startsWith("@") && // `@children slots are matched to the children prop, and should not be handled separately for type-checking
        item.name !== "@children") {
            slots.push(item.name.slice(1));
        }
    }
    return slots;
}
// By exposing the static route types separately as string literals,
// editors can provide autocompletion for them. However it's currently not
// possible to provide the same experience for dynamic routes.
const pluginState = (0, _buildcontext.getProxiedPluginState)({
    routeTypes: {
        edge: {
            static: "",
            dynamic: ""
        },
        node: {
            static: "",
            dynamic: ""
        },
        extra: {
            static: "",
            dynamic: ""
        }
    }
});
function formatRouteToRouteType(route) {
    const isDynamic = (0, _utils.isDynamicRoute)(route);
    if (isDynamic) {
        route = route.split("/").map((part)=>{
            if (part.startsWith("[") && part.endsWith("]")) {
                if (part.startsWith("[...")) {
                    // /[...slug]
                    return `\${CatchAllSlug<T>}`;
                } else if (part.startsWith("[[...") && part.endsWith("]]")) {
                    // /[[...slug]]
                    return `\${OptionalCatchAllSlug<T>}`;
                }
                // /[slug]
                return `\${SafeSlug<T>}`;
            }
            return part;
        }).join("/");
    }
    return {
        isDynamic,
        routeType: `\n    | \`${route}\``
    };
}
// Whether redirects and rewrites have been converted into routeTypes or not.
let redirectsRewritesTypesProcessed = false;
// Convert redirects and rewrites into routeTypes.
function addRedirectsRewritesRouteTypes(rewrites, redirects) {
    function addExtraRoute(source) {
        let tokens;
        try {
            tokens = (0, _pathtoregexp.parse)(source);
        } catch  {
        // Ignore invalid routes - they will be handled by other checks.
        }
        if (Array.isArray(tokens)) {
            const possibleNormalizedRoutes = [
                ""
            ];
            let slugCnt = 1;
            function append(suffix) {
                for(let i = 0; i < possibleNormalizedRoutes.length; i++){
                    possibleNormalizedRoutes[i] += suffix;
                }
            }
            function fork(suffix) {
                const currentLength = possibleNormalizedRoutes.length;
                for(let i = 0; i < currentLength; i++){
                    possibleNormalizedRoutes.push(possibleNormalizedRoutes[i] + suffix);
                }
            }
            for (const token of tokens){
                if (typeof token === "object") {
                    // Make sure the slug is always named.
                    const slug = token.name || (slugCnt++ === 1 ? "slug" : `slug${slugCnt}`);
                    if (token.modifier === "*") {
                        append(`${token.prefix}[[...${slug}]]`);
                    } else if (token.modifier === "+") {
                        append(`${token.prefix}[...${slug}]`);
                    } else if (token.modifier === "") {
                        if (token.pattern === "[^\\/#\\?]+?") {
                            // A safe slug
                            append(`${token.prefix}[${slug}]`);
                        } else if (token.pattern === ".*") {
                            // An optional catch-all slug
                            append(`${token.prefix}[[...${slug}]]`);
                        } else if (token.pattern === ".+") {
                            // A catch-all slug
                            append(`${token.prefix}[...${slug}]`);
                        } else {
                            // Other regex patterns are not supported. Skip this route.
                            return;
                        }
                    } else if (token.modifier === "?") {
                        if (/^[a-zA-Z0-9_/]*$/.test(token.pattern)) {
                            // An optional slug with plain text only, fork the route.
                            append(token.prefix);
                            fork(token.pattern);
                        } else {
                            // Optional modifier `?` and regex patterns are not supported.
                            return;
                        }
                    }
                } else if (typeof token === "string") {
                    append(token);
                }
            }
            for (const normalizedRoute of possibleNormalizedRoutes){
                const { isDynamic, routeType } = formatRouteToRouteType(normalizedRoute);
                pluginState.routeTypes.extra[isDynamic ? "dynamic" : "static"] += routeType;
            }
        }
    }
    if (rewrites) {
        for (const rewrite of rewrites.beforeFiles){
            addExtraRoute(rewrite.source);
        }
        for (const rewrite of rewrites.afterFiles){
            addExtraRoute(rewrite.source);
        }
        for (const rewrite of rewrites.fallback){
            addExtraRoute(rewrite.source);
        }
    }
    if (redirects) {
        for (const redirect of redirects){
            // Skip internal redirects
            // https://github.com/vercel/next.js/blob/8ff3d7ff57836c24088474175d595b4d50b3f857/packages/next/src/lib/load-custom-routes.ts#L704-L710
            if (!("internal" in redirect)) {
                addExtraRoute(redirect.source);
            }
        }
    }
}
function createRouteDefinitions() {
    let staticRouteTypes = "";
    let dynamicRouteTypes = "";
    for (const type of [
        "edge",
        "node",
        "extra"
    ]){
        staticRouteTypes += pluginState.routeTypes[type].static;
        dynamicRouteTypes += pluginState.routeTypes[type].dynamic;
    }
    // If both StaticRoutes and DynamicRoutes are empty, fallback to type 'string'.
    const routeTypesFallback = !staticRouteTypes && !dynamicRouteTypes ? "string" : "";
    return `// Type definitions for Next.js routes

/**
 * Internal types used by the Next.js router and Link component.
 * These types are not meant to be used directly.
 * @internal
 */
declare namespace __next_route_internal_types__ {
  type SearchOrHash = \`?\${string}\` | \`#\${string}\`
  type WithProtocol = \`\${string}:\${string}\`

  type Suffix = '' | SearchOrHash

  type SafeSlug<S extends string> = S extends \`\${string}/\${string}\`
    ? never
    : S extends \`\${string}\${SearchOrHash}\`
    ? never
    : S extends ''
    ? never
    : S

  type CatchAllSlug<S extends string> = S extends \`\${string}\${SearchOrHash}\`
    ? never
    : S extends ''
    ? never
    : S

  type OptionalCatchAllSlug<S extends string> =
    S extends \`\${string}\${SearchOrHash}\` ? never : S

  type StaticRoutes = ${staticRouteTypes || "never"}
  type DynamicRoutes<T extends string = string> = ${dynamicRouteTypes || "never"}

  type RouteImpl<T> = ${routeTypesFallback || `
    ${// This keeps autocompletion working for static routes.
    "| StaticRoutes"}
    | SearchOrHash
    | WithProtocol
    | \`\${StaticRoutes}\${SearchOrHash}\`
    | (T extends \`\${DynamicRoutes<infer _>}\${Suffix}\` ? T : never)
    `}
}

declare module 'next' {
  export { default } from 'next/types/index.js'
  export * from 'next/types/index.js'

  export type Route<T extends string = string> =
    __next_route_internal_types__.RouteImpl<T>
}

declare module 'next/link' {
  import type { LinkProps as OriginalLinkProps } from 'next/dist/client/link.js'
  import type { AnchorHTMLAttributes, DetailedHTMLProps } from 'react'
  import type { UrlObject } from 'url'

  type LinkRestProps = Omit<
    Omit<
      DetailedHTMLProps<
        AnchorHTMLAttributes<HTMLAnchorElement>,
        HTMLAnchorElement
      >,
      keyof OriginalLinkProps
    > &
      OriginalLinkProps,
    'href'
  >

  export type LinkProps<RouteInferType> = LinkRestProps & {
    /**
     * The path or URL to navigate to. This is the only required prop. It can also be an object.
     * @see https://nextjs.org/docs/api-reference/next/link
     */
    href: __next_route_internal_types__.RouteImpl<RouteInferType> | UrlObject
  }

  export default function Link<RouteType>(props: LinkProps<RouteType>): JSX.Element
}

declare module 'next/navigation' {
  export * from 'next/dist/client/components/navigation.js'

  import type { NavigateOptions, AppRouterInstance as OriginalAppRouterInstance } from 'next/dist/shared/lib/app-router-context.shared-runtime.js'
  interface AppRouterInstance extends OriginalAppRouterInstance {
    /**
     * Navigate to the provided href.
     * Pushes a new history entry.
     */
    push<RouteType>(href: __next_route_internal_types__.RouteImpl<RouteType>, options?: NavigateOptions): void
    /**
     * Navigate to the provided href.
     * Replaces the current history entry.
     */
    replace<RouteType>(href: __next_route_internal_types__.RouteImpl<RouteType>, options?: NavigateOptions): void
    /**
     * Prefetch the provided href.
     */
    prefetch<RouteType>(href: __next_route_internal_types__.RouteImpl<RouteType>): void
  }

  export declare function useRouter(): AppRouterInstance;
}
`;
}
const appTypesBasePath = _path.default.join("types", "app");
class NextTypesPlugin {
    constructor(options){
        this.dir = options.dir;
        this.distDir = options.distDir;
        this.appDir = options.appDir;
        this.dev = options.dev;
        this.isEdgeServer = options.isEdgeServer;
        this.pageExtensions = options.pageExtensions;
        this.pagesDir = _path.default.join(this.appDir, "..", "pages");
        this.typedRoutes = options.typedRoutes;
        this.distDirAbsolutePath = _path.default.join(this.dir, this.distDir);
        if (this.typedRoutes && !redirectsRewritesTypesProcessed) {
            redirectsRewritesTypesProcessed = true;
            addRedirectsRewritesRouteTypes(options.originalRewrites, options.originalRedirects);
        }
    }
    getRelativePathFromAppTypesDir(moduleRelativePathToAppDir) {
        const moduleAbsolutePath = _path.default.join(this.appDir, moduleRelativePathToAppDir);
        const moduleInAppTypesAbsolutePath = _path.default.join(this.distDirAbsolutePath, appTypesBasePath, moduleRelativePathToAppDir);
        return _path.default.relative(moduleInAppTypesAbsolutePath + "/..", moduleAbsolutePath);
    }
    collectPage(filePath) {
        if (!this.typedRoutes) return;
        const isApp = filePath.startsWith(this.appDir + _path.default.sep);
        const isPages = !isApp && filePath.startsWith(this.pagesDir + _path.default.sep);
        if (!isApp && !isPages) {
            return;
        }
        // Filter out non-page and non-route files in app dir
        if (isApp && !/[/\\](?:page|route)\.[^.]+$/.test(filePath)) {
            return;
        }
        // Filter out non-page files in pages dir
        if (isPages && /[/\\](?:_app|_document|_error|404|500)\.[^.]+$/.test(filePath)) {
            return;
        }
        let route = (isApp ? _apppaths.normalizeAppPath : _denormalizepagepath.denormalizePagePath)((0, _ensureleadingslash.ensureLeadingSlash)((0, _entries.getPageFromPath)(_path.default.relative(isApp ? this.appDir : this.pagesDir, filePath), this.pageExtensions)));
        const { isDynamic, routeType } = formatRouteToRouteType(route);
        pluginState.routeTypes[this.isEdgeServer ? "edge" : "node"][isDynamic ? "dynamic" : "static"] += routeType;
    }
    apply(compiler) {
        // From asset root to dist root
        const assetDirRelative = this.dev ? ".." : this.isEdgeServer ? ".." : "../..";
        const handleModule = async (mod, assets)=>{
            if (!mod.resource) return;
            if (!/\.(js|jsx|ts|tsx|mjs)$/.test(mod.resource)) return;
            if (!mod.resource.startsWith(this.appDir + _path.default.sep)) {
                if (!this.dev) {
                    if (mod.resource.startsWith(this.pagesDir + _path.default.sep)) {
                        this.collectPage(mod.resource);
                    }
                }
                return;
            }
            if (mod.layer !== _constants.WEBPACK_LAYERS.reactServerComponents && mod.layer !== _constants.WEBPACK_LAYERS.appRouteHandler) return;
            const IS_LAYOUT = /[/\\]layout\.[^./\\]+$/.test(mod.resource);
            const IS_PAGE = !IS_LAYOUT && /[/\\]page\.[^.]+$/.test(mod.resource);
            const IS_ROUTE = !IS_PAGE && /[/\\]route\.[^.]+$/.test(mod.resource);
            const relativePathToApp = _path.default.relative(this.appDir, mod.resource);
            if (!this.dev) {
                if (IS_PAGE || IS_ROUTE) {
                    this.collectPage(mod.resource);
                }
            }
            const typePath = _path.default.join(appTypesBasePath, relativePathToApp.replace(/\.(js|jsx|ts|tsx|mjs)$/, ".ts"));
            const relativeImportPath = (0, _normalizepathsep.normalizePathSep)(_path.default.join(this.getRelativePathFromAppTypesDir(relativePathToApp)).replace(/\.(js|jsx|ts|tsx|mjs)$/, ""));
            const assetPath = _path.default.join(assetDirRelative, typePath);
            if (IS_LAYOUT) {
                const slots = await collectNamedSlots(mod.resource);
                assets[assetPath] = new _webpack.sources.RawSource(createTypeGuardFile(mod.resource, relativeImportPath, {
                    type: "layout",
                    slots
                }));
            } else if (IS_PAGE) {
                assets[assetPath] = new _webpack.sources.RawSource(createTypeGuardFile(mod.resource, relativeImportPath, {
                    type: "page"
                }));
            } else if (IS_ROUTE) {
                assets[assetPath] = new _webpack.sources.RawSource(createTypeGuardFile(mod.resource, relativeImportPath, {
                    type: "route"
                }));
            }
        };
        compiler.hooks.compilation.tap(PLUGIN_NAME, (compilation)=>{
            compilation.hooks.processAssets.tapAsync({
                name: PLUGIN_NAME,
                stage: _webpack.webpack.Compilation.PROCESS_ASSETS_STAGE_OPTIMIZE_HASH
            }, async (assets, callback)=>{
                const promises = [];
                // Clear routes
                if (this.isEdgeServer) {
                    pluginState.routeTypes.edge.dynamic = "";
                    pluginState.routeTypes.edge.static = "";
                } else {
                    pluginState.routeTypes.node.dynamic = "";
                    pluginState.routeTypes.node.static = "";
                }
                compilation.chunkGroups.forEach((chunkGroup)=>{
                    chunkGroup.chunks.forEach((chunk)=>{
                        if (!chunk.name) return;
                        // Here we only track page and route chunks.
                        if (!chunk.name.startsWith("pages/") && !(chunk.name.startsWith("app/") && (chunk.name.endsWith("/page") || chunk.name.endsWith("/route")))) {
                            return;
                        }
                        const chunkModules = compilation.chunkGraph.getChunkModulesIterable(chunk);
                        for (const mod of chunkModules){
                            promises.push(handleModule(mod, assets));
                            // If this is a concatenation, register each child to the parent ID.
                            const anyModule = mod;
                            if (anyModule.modules) {
                                anyModule.modules.forEach((concatenatedMod)=>{
                                    promises.push(handleModule(concatenatedMod, assets));
                                });
                            }
                        }
                    });
                });
                await Promise.all(promises);
                // Support `"moduleResolution": "Node16" | "NodeNext"` with `"type": "module"`
                const packageJsonAssetPath = _path.default.join(assetDirRelative, "types/package.json");
                assets[packageJsonAssetPath] = new _webpack.sources.RawSource('{"type": "module"}');
                if (this.typedRoutes) {
                    if (this.dev && !this.isEdgeServer) {
                        _shared.devPageFiles.forEach((file)=>{
                            this.collectPage(file);
                        });
                    }
                    const linkAssetPath = _path.default.join(assetDirRelative, "types/link.d.ts");
                    assets[linkAssetPath] = new _webpack.sources.RawSource(createRouteDefinitions());
                }
                callback();
            });
        });
    }
}

//# sourceMappingURL=index.js.map