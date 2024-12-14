/*
The MIT License (MIT)

Copyright (c) 2016 Ben Holloway

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, /**
 * A webpack loader that resolves absolute url() paths relative to their original source file.
 * Requires source-maps to do any meaningful work.
 */ "default", {
    enumerable: true,
    get: function() {
        return resolveUrlLoader;
    }
});
const _sourcemap = require("next/dist/compiled/source-map");
const _valueprocessor = /*#__PURE__*/ _interop_require_default(require("./lib/value-processor"));
const _joinfunction = require("./lib/join-function");
const _postcss = /*#__PURE__*/ _interop_require_default(require("./lib/postcss"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function resolveUrlLoader(/** Css content */ content, /** The source-map */ sourceMap) {
    const options = Object.assign({
        sourceMap: this.sourceMap,
        silent: false,
        absolute: false,
        keepQuery: false,
        root: false,
        debug: false,
        join: _joinfunction.defaultJoin
    }, this.getOptions());
    let sourceMapConsumer;
    if (sourceMap) {
        sourceMapConsumer = new _sourcemap.SourceMapConsumer(sourceMap);
    }
    const callback = this.async();
    const { postcss } = options.postcss ? await options.postcss() : {
        postcss: require('postcss')
    };
    (0, _postcss.default)(postcss, this.resourcePath, content, {
        outputSourceMap: Boolean(options.sourceMap),
        transformDeclaration: (0, _valueprocessor.default)(this.resourcePath, options),
        inputSourceMap: sourceMap,
        sourceMapConsumer: sourceMapConsumer
    })// eslint-disable-next-line @typescript-eslint/no-use-before-define
    .catch(onFailure)// eslint-disable-next-line @typescript-eslint/no-use-before-define
    .then(onSuccess);
    function onFailure(error) {
        // eslint-disable-next-line @typescript-eslint/no-use-before-define
        callback(encodeError('CSS error', error));
    }
    function onSuccess(reworked) {
        if (reworked) {
            // complete with source-map
            //  source-map sources are relative to the file being processed
            if (options.sourceMap) {
                callback(null, reworked.content, reworked.map);
            } else {
                callback(null, reworked.content);
            }
        }
    }
    function encodeError(label, exception) {
        return new Error([
            'resolve-url-loader',
            ': ',
            [
                label
            ].concat(typeof exception === 'string' && exception || exception instanceof Error && [
                exception.message,
                exception.stack.split('\n', 2)[1].trim()
            ] || []).filter(Boolean).join('\n  ')
        ].join(''));
    }
}

//# sourceMappingURL=index.js.map