import isError from "../../lib/is-error";
const hydrationErrorRegex = /hydration failed|while hydrating|content does not match|did not match/i;
export function isHydrationError(error) {
    return isError(error) && hydrationErrorRegex.test(error.message);
}

//# sourceMappingURL=is-hydration-error.js.map