import { isNotFoundError } from "./not-found";
import { isRedirectError } from "./redirect";
export function isNextRouterError(error) {
    return error && error.digest && (isRedirectError(error) || isNotFoundError(error));
}

//# sourceMappingURL=is-next-router-error.js.map