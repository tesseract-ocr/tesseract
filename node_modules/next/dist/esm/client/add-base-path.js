import { addPathPrefix } from "../shared/lib/router/utils/add-path-prefix";
import { normalizePathTrailingSlash } from "./normalize-trailing-slash";
const basePath = process.env.__NEXT_ROUTER_BASEPATH || "";
export function addBasePath(path, required) {
    return normalizePathTrailingSlash(process.env.__NEXT_MANUAL_CLIENT_BASE_PATH && !required ? path : addPathPrefix(path, basePath));
}

//# sourceMappingURL=add-base-path.js.map