"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, // Handle module.exports in user code
"default", {
    enumerable: true,
    get: function() {
        return CommonJSModulePlugin;
    }
});
const _plugintransformmodulescommonjs = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/babel/plugin-transform-modules-commonjs"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function CommonJSModulePlugin(...args) {
    const commonjs = (0, _plugintransformmodulescommonjs.default)(...args);
    return {
        visitor: {
            Program: {
                exit (path, state) {
                    let foundModuleExports = false;
                    path.traverse({
                        MemberExpression (expressionPath) {
                            if (expressionPath.node.object.name !== "module") return;
                            if (expressionPath.node.property.name !== "exports") return;
                            foundModuleExports = true;
                        }
                    });
                    if (!foundModuleExports) {
                        return;
                    }
                    commonjs.visitor.Program.exit.call(this, path, state);
                }
            }
        }
    };
}

//# sourceMappingURL=commonjs.js.map