import { matchHas } from './prepare-destination';
export function getMiddlewareRouteMatcher(matchers) {
    return (pathname, req, query)=>{
        for (const matcher of matchers){
            const routeMatch = new RegExp(matcher.regexp).exec(pathname);
            if (!routeMatch) {
                continue;
            }
            if (matcher.has || matcher.missing) {
                const hasParams = matchHas(req, query, matcher.has, matcher.missing);
                if (!hasParams) {
                    continue;
                }
            }
            return true;
        }
        return false;
    };
}

//# sourceMappingURL=middleware-route-matcher.js.map