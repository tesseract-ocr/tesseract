import { hasBasePath } from './has-base-path';
const basePath = process.env.__NEXT_ROUTER_BASEPATH || '';
export function removeBasePath(path) {
    if (process.env.__NEXT_MANUAL_CLIENT_BASE_PATH) {
        if (!hasBasePath(path)) {
            return path;
        }
    }
    // Can't trim the basePath if it has zero length!
    if (basePath.length === 0) return path;
    path = path.slice(basePath.length);
    if (!path.startsWith('/')) path = "/" + path;
    return path;
}

//# sourceMappingURL=remove-base-path.js.map