/**
 * This transforms a URL pathname into a route. It removes any trailing slashes
 * and the `/index` suffix.
 *
 * @param {string} pathname - The URL path that needs to be optimized.
 * @returns {string} - The route
 *
 * @example
 * // returns '/example'
 * toRoute('/example/index/');
 *
 * @example
 * // returns '/example'
 * toRoute('/example/');
 *
 * @example
 * // returns '/'
 * toRoute('/index/');
 *
 * @example
 * // returns '/'
 * toRoute('/');
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "toRoute", {
    enumerable: true,
    get: function() {
        return toRoute;
    }
});
function toRoute(pathname) {
    return pathname.replace(/(?:\/index)?\/?$/, '') || '/';
}

//# sourceMappingURL=to-route.js.map