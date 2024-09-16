"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "parseModule", {
    enumerable: true,
    get: function() {
        return parseModule;
    }
});
const _lrucache = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/lru-cache"));
const _withpromisecache = require("../../lib/with-promise-cache");
const _crypto = require("crypto");
const _swc = require("../swc");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const parseModule = (0, _withpromisecache.withPromiseCache)(new _lrucache.default({
    max: 500
}), async (filename, content)=>(0, _swc.parse)(content, {
        isModule: "unknown",
        filename
    }).catch(()=>null), (_, content)=>(0, _crypto.createHash)("sha1").update(content).digest("hex"));

//# sourceMappingURL=parse-module.js.map