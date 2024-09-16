"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const ReactRefreshModule_runtime_1 = __importDefault(require("./internal/ReactRefreshModule.runtime"));
let refreshModuleRuntime = ReactRefreshModule_runtime_1.default.toString();
refreshModuleRuntime = refreshModuleRuntime
    .slice(refreshModuleRuntime.indexOf('{') + 1, refreshModuleRuntime.lastIndexOf('}'))
    // Given that the import above executes the module we need to make sure it does not crash on `import.meta` not being allowed.
    .replace('global.importMeta', 'import.meta');
let commonJsrefreshModuleRuntime = refreshModuleRuntime.replace('import.meta.webpackHot', 'module.hot');
const ReactRefreshLoader = function ReactRefreshLoader(source, inputSourceMap) {
    this.callback(null, `${source}\n\n;${
    // Account for commonjs not supporting `import.meta
    this.resourcePath.endsWith('.cjs')
        ? commonJsrefreshModuleRuntime
        : refreshModuleRuntime}`, inputSourceMap);
};
exports.default = ReactRefreshLoader;
//# sourceMappingURL=loader.js.map