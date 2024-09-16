"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "base", {
    enumerable: true,
    get: function() {
        return base;
    }
});
const _lodashcurry = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/lodash.curry"));
const _constants = require("../../../../shared/lib/constants");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const base = (0, _lodashcurry.default)(function base(ctx, config) {
    config.mode = ctx.isDevelopment ? "development" : "production";
    config.name = ctx.isServer ? ctx.isEdgeRuntime ? _constants.COMPILER_NAMES.edgeServer : _constants.COMPILER_NAMES.server : _constants.COMPILER_NAMES.client;
    config.target = !ctx.targetWeb ? "node18.17" // Same version defined in packages/next/package.json#engines
     : ctx.isEdgeRuntime ? [
        "web",
        "es6"
    ] : [
        "web",
        "es5"
    ];
    // https://webpack.js.org/configuration/devtool/#development
    if (ctx.isDevelopment) {
        if (process.env.__NEXT_TEST_MODE && !process.env.__NEXT_TEST_WITH_DEVTOOL) {
            config.devtool = false;
        } else {
            // `eval-source-map` provides full-fidelity source maps for the
            // original source, including columns and original variable names.
            // This is desirable so the in-browser debugger can correctly pause
            // and show scoped variables with their original names.
            config.devtool = "eval-source-map";
        }
    } else {
        if (ctx.isEdgeRuntime || ctx.isServer && ctx.serverSourceMaps || // Enable browser sourcemaps:
        ctx.productionBrowserSourceMaps && ctx.isClient) {
            config.devtool = "source-map";
        } else {
            config.devtool = false;
        }
    }
    if (!config.module) {
        config.module = {
            rules: []
        };
    }
    // TODO: add codemod for "Should not import the named export" with JSON files
    // config.module.strictExportPresence = !isWebpack5
    return config;
});

//# sourceMappingURL=base.js.map