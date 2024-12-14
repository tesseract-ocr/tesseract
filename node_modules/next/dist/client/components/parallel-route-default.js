"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    PARALLEL_ROUTE_DEFAULT_PATH: null,
    default: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    PARALLEL_ROUTE_DEFAULT_PATH: function() {
        return PARALLEL_ROUTE_DEFAULT_PATH;
    },
    default: function() {
        return ParallelRouteDefault;
    }
});
const _notfound = require("./not-found");
const PARALLEL_ROUTE_DEFAULT_PATH = 'next/dist/client/components/parallel-route-default.js';
function ParallelRouteDefault() {
    (0, _notfound.notFound)();
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=parallel-route-default.js.map