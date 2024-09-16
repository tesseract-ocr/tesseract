/*
 * Partially adapted from @babel/core (MIT license).
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return transform;
    }
});
const _traverse = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/babel/traverse"));
const _generator = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/babel/generator"));
const _corelibnormalizefile = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/babel/core-lib-normalize-file"));
const _corelibnormalizeopts = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/babel/core-lib-normalize-opts"));
const _corelibblockhoistplugin = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/babel/core-lib-block-hoist-plugin"));
const _corelibpluginpass = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/babel/core-lib-plugin-pass"));
const _getconfig = /*#__PURE__*/ _interop_require_default(require("./get-config"));
const _util = require("./util");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getTraversalParams(file, pluginPairs) {
    const passPairs = [];
    const passes = [];
    const visitors = [];
    for (const plugin of pluginPairs.concat((0, _corelibblockhoistplugin.default)())){
        const pass = new _corelibpluginpass.default(file, plugin.key, plugin.options);
        passPairs.push([
            plugin,
            pass
        ]);
        passes.push(pass);
        visitors.push(plugin.visitor);
    }
    return {
        passPairs,
        passes,
        visitors
    };
}
function invokePluginPre(file, passPairs) {
    for (const [{ pre }, pass] of passPairs){
        if (pre) {
            pre.call(pass, file);
        }
    }
}
function invokePluginPost(file, passPairs) {
    for (const [{ post }, pass] of passPairs){
        if (post) {
            post.call(pass, file);
        }
    }
}
function transformAstPass(file, pluginPairs, parentSpan) {
    const { passPairs, passes, visitors } = getTraversalParams(file, pluginPairs);
    invokePluginPre(file, passPairs);
    const visitor = _traverse.default.visitors.merge(visitors, passes, // @ts-ignore - the exported types are incorrect here
    file.opts.wrapPluginVisitorMethod);
    parentSpan.traceChild("babel-turbo-traverse").traceFn(()=>(0, _traverse.default)(file.ast, visitor, file.scope));
    invokePluginPost(file, passPairs);
}
function transformAst(file, babelConfig, parentSpan) {
    for (const pluginPairs of babelConfig.passes){
        transformAstPass(file, pluginPairs, parentSpan);
    }
}
function transform(source, inputSourceMap, loaderOptions, filename, target, parentSpan) {
    const getConfigSpan = parentSpan.traceChild("babel-turbo-get-config");
    const babelConfig = _getconfig.default.call(this, {
        source,
        loaderOptions,
        inputSourceMap,
        target,
        filename
    });
    getConfigSpan.stop();
    const normalizeSpan = parentSpan.traceChild("babel-turbo-normalize-file");
    const file = (0, _util.consumeIterator)((0, _corelibnormalizefile.default)(babelConfig.passes, (0, _corelibnormalizeopts.default)(babelConfig), source));
    normalizeSpan.stop();
    const transformSpan = parentSpan.traceChild("babel-turbo-transform");
    transformAst(file, babelConfig, transformSpan);
    transformSpan.stop();
    const generateSpan = parentSpan.traceChild("babel-turbo-generate");
    const { code, map } = (0, _generator.default)(file.ast, file.opts.generatorOpts, file.code);
    generateSpan.stop();
    return {
        code,
        map
    };
}

//# sourceMappingURL=transform.js.map