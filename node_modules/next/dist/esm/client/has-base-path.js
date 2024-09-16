import { pathHasPrefix } from "../shared/lib/router/utils/path-has-prefix";
const basePath = process.env.__NEXT_ROUTER_BASEPATH || "";
export function hasBasePath(path) {
    return pathHasPrefix(path, basePath);
}

//# sourceMappingURL=has-base-path.js.map