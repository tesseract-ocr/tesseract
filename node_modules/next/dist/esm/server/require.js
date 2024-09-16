import path from "path";
import { AUTOMATIC_FONT_OPTIMIZATION_MANIFEST, PAGES_MANIFEST, SERVER_DIRECTORY, APP_PATHS_MANIFEST } from "../shared/lib/constants";
import { normalizeLocalePath } from "../shared/lib/i18n/normalize-locale-path";
import { normalizePagePath } from "../shared/lib/page-path/normalize-page-path";
import { denormalizePagePath } from "../shared/lib/page-path/denormalize-page-path";
import { PageNotFoundError, MissingStaticPage } from "../shared/lib/utils";
import LRUCache from "next/dist/compiled/lru-cache";
import { loadManifest } from "./load-manifest";
import { promises } from "fs";
const isDev = process.env.NODE_ENV === "development";
const pagePathCache = !isDev ? new LRUCache({
    max: 1000
}) : null;
export function getMaybePagePath(page, distDir, locales, isAppPath) {
    const cacheKey = `${page}:${distDir}:${locales}:${isAppPath}`;
    let pagePath = pagePathCache == null ? void 0 : pagePathCache.get(cacheKey);
    // If we have a cached path, we can return it directly.
    if (pagePath) return pagePath;
    const serverBuildPath = path.join(distDir, SERVER_DIRECTORY);
    let appPathsManifest;
    if (isAppPath) {
        appPathsManifest = loadManifest(path.join(serverBuildPath, APP_PATHS_MANIFEST), !isDev);
    }
    const pagesManifest = loadManifest(path.join(serverBuildPath, PAGES_MANIFEST), !isDev);
    try {
        page = denormalizePagePath(normalizePagePath(page));
    } catch (err) {
        console.error(err);
        throw new PageNotFoundError(page);
    }
    const checkManifest = (manifest)=>{
        let curPath = manifest[page];
        if (!manifest[curPath] && locales) {
            const manifestNoLocales = {};
            for (const key of Object.keys(manifest)){
                manifestNoLocales[normalizeLocalePath(key, locales).pathname] = pagesManifest[key];
            }
            curPath = manifestNoLocales[page];
        }
        return curPath;
    };
    if (appPathsManifest) {
        pagePath = checkManifest(appPathsManifest);
    }
    if (!pagePath) {
        pagePath = checkManifest(pagesManifest);
    }
    if (!pagePath) {
        pagePathCache == null ? void 0 : pagePathCache.set(cacheKey, null);
        return null;
    }
    pagePath = path.join(serverBuildPath, pagePath);
    pagePathCache == null ? void 0 : pagePathCache.set(cacheKey, pagePath);
    return pagePath;
}
export function getPagePath(page, distDir, locales, isAppPath) {
    const pagePath = getMaybePagePath(page, distDir, locales, isAppPath);
    if (!pagePath) {
        throw new PageNotFoundError(page);
    }
    return pagePath;
}
export function requirePage(page, distDir, isAppPath) {
    const pagePath = getPagePath(page, distDir, undefined, isAppPath);
    if (pagePath.endsWith(".html")) {
        return promises.readFile(pagePath, "utf8").catch((err)=>{
            throw new MissingStaticPage(page, err.message);
        });
    }
    // since require is synchronous we can set the specific runtime
    // we are requiring for the require-hook and then clear after
    try {
        process.env.__NEXT_PRIVATE_RUNTIME_TYPE = isAppPath ? "app" : "pages";
        const mod = process.env.NEXT_MINIMAL ? __non_webpack_require__(pagePath) : require(pagePath);
        return mod;
    } finally{
        process.env.__NEXT_PRIVATE_RUNTIME_TYPE = "";
    }
}
export function requireFontManifest(distDir) {
    const serverBuildPath = path.join(distDir, SERVER_DIRECTORY);
    const fontManifest = loadManifest(path.join(serverBuildPath, AUTOMATIC_FONT_OPTIMIZATION_MANIFEST));
    return fontManifest;
}

//# sourceMappingURL=require.js.map