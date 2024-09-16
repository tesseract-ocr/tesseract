"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, /**
 * This loader is to create special re-exports from a specific file.
 * For example, the following loader:
 *
 * modularize-import-loader?name=Arrow&from=Arrow&as=default&join=./icons/Arrow!lucide-react
 *
 * will be used to create a re-export of:
 *
 * export { Arrow as default } from "join(resolve_path('lucide-react'), '/icons/Arrow')"
 *
 * This works even if there's no export field in the package.json of the package.
 */ "default", {
    enumerable: true,
    get: function() {
        return transformSource;
    }
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function transformSource() {
    const { name, from, as, join } = this.getOptions();
    const { resourcePath } = this;
    const fullPath = join ? _path.default.join(_path.default.dirname(resourcePath), join) : resourcePath;
    return `
export {
  ${from === "default" ? "default" : name} as ${as === "default" ? "default" : name}
} from ${JSON.stringify(fullPath)}
`;
}

//# sourceMappingURL=modularize-import-loader.js.map