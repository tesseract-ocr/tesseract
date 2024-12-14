"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
require("../../server/web/globals");
const _adapter = require("../../server/web/adapter");
const _incrementalcache = require("../../server/lib/incremental-cache");
const _apiutils = require("../../server/api-utils");
const _VAR_USERLAND = /*#__PURE__*/ _interop_require_default(require("VAR_USERLAND"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const page = 'VAR_DEFINITION_PAGE';
if (typeof _VAR_USERLAND.default !== 'function') {
    throw new Error(`The Edge Function "pages${page}" must export a \`default\` function`);
}
function _default(opts) {
    return (0, _adapter.adapter)({
        ...opts,
        IncrementalCache: _incrementalcache.IncrementalCache,
        page: 'VAR_DEFINITION_PATHNAME',
        handler: (0, _apiutils.wrapApiHandler)(page, _VAR_USERLAND.default)
    });
}

//# sourceMappingURL=pages-edge-api.js.map