"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "URLPattern", {
    enumerable: true,
    get: function() {
        return GlobalURLPattern;
    }
});
const GlobalURLPattern = // @ts-expect-error: URLPattern is not available in Node.js
typeof URLPattern === 'undefined' ? undefined : URLPattern;

//# sourceMappingURL=url-pattern.js.map