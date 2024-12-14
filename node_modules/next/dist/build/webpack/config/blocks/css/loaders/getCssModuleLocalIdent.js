"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getCssModuleLocalIdent", {
    enumerable: true,
    get: function() {
        return getCssModuleLocalIdent;
    }
});
const _loaderutils3 = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/loader-utils3"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const regexLikeIndexModule = /(?<!pages[\\/])index\.module\.(scss|sass|css)$/;
function getCssModuleLocalIdent(context, _, exportName, options) {
    const relativePath = _path.default.relative(context.rootContext, context.resourcePath).replace(/\\+/g, '/');
    // Generate a more meaningful name (parent folder) when the user names the
    // file `index.module.css`.
    const fileNameOrFolder = regexLikeIndexModule.test(relativePath) ? '[folder]' : '[name]';
    // Generate a hash to make the class name unique.
    const hash = _loaderutils3.default.getHashDigest(Buffer.from(`filePath:${relativePath}#className:${exportName}`), 'sha1', 'base64', 5);
    // Have webpack interpolate the `[folder]` or `[name]` to its real value.
    return _loaderutils3.default.interpolateName(context, fileNameOrFolder + '_' + exportName + '__' + hash, options).replace(// Webpack name interpolation returns `about.module_root__2oFM9` for
    // `.root {}` inside a file named `about.module.css`. Let's simplify
    // this.
    /\.module_/, '_')// Replace invalid symbols with underscores instead of escaping
    // https://mathiasbynens.be/notes/css-escapes#identifiers-strings
    .replace(/[^a-zA-Z0-9-_]/g, '_')// "they cannot start with a digit, two hyphens, or a hyphen followed by a digit [sic]"
    // https://www.w3.org/TR/CSS21/syndata.html#characters
    .replace(/^(\d|--|-\d)/, '__$1');
}

//# sourceMappingURL=getCssModuleLocalIdent.js.map