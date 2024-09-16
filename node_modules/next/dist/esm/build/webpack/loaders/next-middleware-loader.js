import { getModuleBuildInfo } from "./get-module-build-info";
import { MIDDLEWARE_LOCATION_REGEXP } from "../../../lib/constants";
import { loadEntrypoint } from "../../load-entrypoint";
// matchers can have special characters that break the loader params
// parsing so we base64 encode/decode the string
export function encodeMatchers(matchers) {
    return Buffer.from(JSON.stringify(matchers)).toString("base64");
}
export function decodeMatchers(encodedMatchers) {
    return JSON.parse(Buffer.from(encodedMatchers, "base64").toString());
}
export default async function middlewareLoader() {
    const { absolutePagePath, page, rootDir, matchers: encodedMatchers, preferredRegion, middlewareConfig: middlewareConfigBase64 } = this.getOptions();
    const matchers = encodedMatchers ? decodeMatchers(encodedMatchers) : undefined;
    const pagePath = this.utils.contextify(this.context || this.rootContext, absolutePagePath);
    const middlewareConfig = JSON.parse(Buffer.from(middlewareConfigBase64, "base64").toString());
    const buildInfo = getModuleBuildInfo(this._module);
    buildInfo.nextEdgeMiddleware = {
        matchers,
        page: page.replace(new RegExp(`/${MIDDLEWARE_LOCATION_REGEXP}$`), "") || "/"
    };
    buildInfo.rootDir = rootDir;
    buildInfo.route = {
        page,
        absolutePagePath,
        preferredRegion,
        middlewareConfig
    };
    return await loadEntrypoint("middleware", {
        VAR_USERLAND: pagePath,
        VAR_DEFINITION_PAGE: page
    });
}

//# sourceMappingURL=next-middleware-loader.js.map