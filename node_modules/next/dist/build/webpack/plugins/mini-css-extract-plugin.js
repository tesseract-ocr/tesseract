// @ts-ignore: TODO: remove when webpack 5 is stable
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return NextMiniCssExtractPlugin;
    }
});
const _minicssextractplugin = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/mini-css-extract-plugin"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
class NextMiniCssExtractPlugin extends _minicssextractplugin.default {
    constructor(...args){
        super(...args), this.__next_css_remove = true;
    }
}

//# sourceMappingURL=mini-css-extract-plugin.js.map