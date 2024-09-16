"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "images", {
    enumerable: true,
    get: function() {
        return images;
    }
});
const _lodashcurry = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/lodash.curry"));
const _webpackconfig = require("../../../../webpack-config");
const _helpers = require("../../helpers");
const _utils = require("../../utils");
const _messages = require("./messages");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const images = (0, _lodashcurry.default)(async function images(_ctx, config) {
    const fns = [
        (0, _helpers.loader)({
            oneOf: [
                {
                    test: _webpackconfig.nextImageLoaderRegex,
                    use: {
                        loader: "error-loader",
                        options: {
                            reason: (0, _messages.getCustomDocumentImageError)()
                        }
                    },
                    issuer: /pages[\\/]_document\./
                }
            ]
        })
    ];
    const fn = (0, _utils.pipe)(...fns);
    return fn(config);
});

//# sourceMappingURL=index.js.map