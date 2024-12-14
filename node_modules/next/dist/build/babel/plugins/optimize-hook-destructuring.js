"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
// matches any hook-like (the default)
const isHook = /^use[A-Z]/;
// matches only built-in hooks provided by React et al
const isBuiltInHook = /^use(Callback|Context|DebugValue|Effect|ImperativeHandle|LayoutEffect|Memo|Reducer|Ref|State)$/;
function _default({ types: t }) {
    const visitor = {
        CallExpression (path, state) {
            const onlyBuiltIns = state.opts.onlyBuiltIns;
            // if specified, options.lib is a list of libraries that provide hook functions
            const libs = state.opts.lib && (state.opts.lib === true ? [
                'react',
                'preact/hooks'
            ] : [].concat(state.opts.lib));
            // skip function calls that are not the init of a variable declaration:
            if (!t.isVariableDeclarator(path.parent)) return;
            // skip function calls where the return value is not Array-destructured:
            if (!t.isArrayPattern(path.parent.id)) return;
            // name of the (hook) function being called:
            const hookName = path.node.callee.name;
            if (libs) {
                const binding = path.scope.getBinding(hookName);
                // not an import
                if (!binding || binding.kind !== 'module') return;
                const specifier = binding.path.parent.source.value;
                // not a match
                if (!libs.some((lib)=>lib === specifier)) return;
            }
            // only match function calls with names that look like a hook
            if (!(onlyBuiltIns ? isBuiltInHook : isHook).test(hookName)) return;
            path.parent.id = t.objectPattern(path.parent.id.elements.reduce((patterns, element, i)=>{
                if (element === null) {
                    return patterns;
                }
                return patterns.concat(t.objectProperty(t.numericLiteral(i), // TODO: fix this
                element));
            }, []));
        }
    };
    return {
        name: 'optimize-hook-destructuring',
        visitor: {
            // this is a workaround to run before preset-env destroys destructured assignments
            Program (path, state) {
                path.traverse(visitor, state);
            }
        }
    };
}

//# sourceMappingURL=optimize-hook-destructuring.js.map