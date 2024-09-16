// TODO: Remove use of `any` type.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
require("./webpack");
const _ = require("./");
const _hotmiddlewareclient = /*#__PURE__*/ _interop_require_default._(require("./dev/hot-middleware-client"));
const _pagebootstrap = require("./page-bootstrap");
require("./setup-hydration-warning");
window.next = {
    version: _.version,
    // router is initialized later so it has to be live-binded
    get router () {
        return _.router;
    },
    emitter: _.emitter
};
const devClient = (0, _hotmiddlewareclient.default)("webpack");
(0, _.initialize)({
    devClient
}).then((param)=>{
    let { assetPrefix } = param;
    return (0, _pagebootstrap.pageBootrap)(assetPrefix);
}).catch((err)=>{
    console.error("Error was not caught", err);
});

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=next-dev.js.map