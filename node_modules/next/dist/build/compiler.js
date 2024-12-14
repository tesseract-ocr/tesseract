"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "runCompiler", {
    enumerable: true,
    get: function() {
        return runCompiler;
    }
});
const _webpack = require("next/dist/compiled/webpack/webpack");
function generateStats(result, stat) {
    const { errors, warnings } = stat.toJson({
        preset: 'errors-warnings',
        moduleTrace: true
    });
    if (errors && errors.length > 0) {
        result.errors.push(...errors);
    }
    if (warnings && warnings.length > 0) {
        result.warnings.push(...warnings);
    }
    return result;
}
// Webpack 5 requires the compiler to be closed (to save caches)
// Webpack 4 does not have this close method so in order to be backwards compatible we check if it exists
function closeCompiler(compiler) {
    return new Promise((resolve, reject)=>{
        // @ts-ignore Close only exists on the compiler in webpack 5
        return compiler.close((err)=>err ? reject(err) : resolve());
    });
}
function runCompiler(config, { runWebpackSpan, inputFileSystem }) {
    return new Promise((resolve, reject)=>{
        const compiler = (0, _webpack.webpack)(config);
        // Ensure we use the previous inputFileSystem
        if (inputFileSystem) {
            compiler.inputFileSystem = inputFileSystem;
        }
        compiler.fsStartTime = Date.now();
        compiler.run((err, stats)=>{
            const webpackCloseSpan = runWebpackSpan.traceChild('webpack-close', {
                name: config.name || 'unknown'
            });
            webpackCloseSpan.traceAsyncFn(()=>closeCompiler(compiler)).then(()=>{
                if (err) {
                    const reason = err.stack ?? err.toString();
                    if (reason) {
                        return resolve([
                            {
                                errors: [
                                    {
                                        message: reason,
                                        details: err.details
                                    }
                                ],
                                warnings: [],
                                stats
                            },
                            compiler.inputFileSystem
                        ]);
                    }
                    return reject(err);
                } else if (!stats) throw new Error('No Stats from webpack');
                const result = webpackCloseSpan.traceChild('webpack-generate-error-stats').traceFn(()=>generateStats({
                        errors: [],
                        warnings: [],
                        stats
                    }, stats));
                return resolve([
                    result,
                    compiler.inputFileSystem
                ]);
            });
        });
    });
}

//# sourceMappingURL=compiler.js.map