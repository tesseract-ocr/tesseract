"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    LightningCssMinifyPlugin: null,
    default: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    LightningCssMinifyPlugin: function() {
        return _minify.LightningCssMinifyPlugin;
    },
    default: function() {
        return _default;
    }
});
const _loader = require("./loader");
const _minify = require("./minify");
const _default = _loader.LightningCssLoader;

//# sourceMappingURL=index.js.map