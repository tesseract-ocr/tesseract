"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, /**
 * **PostCSS Loader**
 *
 * Loads && processes CSS with [PostCSS](https://github.com/postcss/postcss)
 */ "default", {
    enumerable: true,
    get: function() {
        return loader;
    }
});
const _Warning = /*#__PURE__*/ _interop_require_default(require("./Warning"));
const _Error = /*#__PURE__*/ _interop_require_default(require("./Error"));
const _utils = require("./utils");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function loader(/** Source */ content, /** Source Map */ sourceMap, meta) {
    const loaderSpan = this.currentTraceSpan.traceChild("postcss-loader");
    const callback = this.async();
    loaderSpan.traceAsyncFn(async ()=>{
        const options = this.getOptions();
        const file = this.resourcePath;
        const useSourceMap = typeof options.sourceMap !== "undefined" ? options.sourceMap : this.sourceMap;
        const processOptions = {
            from: file,
            to: file
        };
        if (useSourceMap) {
            processOptions.map = {
                inline: false,
                annotation: false,
                ...processOptions.map
            };
        }
        if (sourceMap && processOptions.map) {
            processOptions.map.prev = loaderSpan.traceChild("normalize-source-map").traceFn(()=>(0, _utils.normalizeSourceMap)(sourceMap, this.context));
        }
        let root;
        // Reuse PostCSS AST from other loaders
        if (meta && meta.ast && meta.ast.type === "postcss") {
            ({ root } = meta.ast);
            loaderSpan.setAttribute("astUsed", "true");
        }
        // Initializes postcss with plugins
        const { postcssWithPlugins } = await options.postcss();
        let result;
        try {
            result = await loaderSpan.traceChild("postcss-process").traceAsyncFn(()=>postcssWithPlugins.process(root || content, processOptions));
        } catch (error) {
            if (error.file) {
                this.addDependency(error.file);
            }
            if (error.name === "CssSyntaxError") {
                throw new _Error.default(error);
            }
            throw error;
        }
        for (const warning of result.warnings()){
            this.emitWarning(new _Warning.default(warning));
        }
        for (const message of result.messages){
            // eslint-disable-next-line default-case
            switch(message.type){
                case "dependency":
                    this.addDependency(message.file);
                    break;
                case "build-dependency":
                    this.addBuildDependency(message.file);
                    break;
                case "missing-dependency":
                    this.addMissingDependency(message.file);
                    break;
                case "context-dependency":
                    this.addContextDependency(message.file);
                    break;
                case "dir-dependency":
                    this.addContextDependency(message.dir);
                    break;
                case "asset":
                    if (message.content && message.file) {
                        this.emitFile(message.file, message.content, message.sourceMap, message.info);
                    }
            }
        }
        // eslint-disable-next-line no-undefined
        let map = result.map ? result.map.toJSON() : undefined;
        if (map && useSourceMap) {
            map = (0, _utils.normalizeSourceMapAfterPostcss)(map, this.context);
        }
        const ast = {
            type: "postcss",
            version: result.processor.version,
            root: result.root
        };
        return [
            result.css,
            map,
            {
                ast
            }
        ];
    }).then(([css, map, { ast }])=>{
        callback == null ? void 0 : callback(null, css, map, {
            ast
        });
    }, (err)=>{
        callback == null ? void 0 : callback(err);
    });
}

//# sourceMappingURL=index.js.map