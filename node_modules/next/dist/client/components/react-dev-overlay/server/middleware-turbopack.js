"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    batchedTraceSource: null,
    getOverlayMiddleware: null,
    getSourceMapMiddleware: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    batchedTraceSource: function() {
        return batchedTraceSource;
    },
    getOverlayMiddleware: function() {
        return getOverlayMiddleware;
    },
    getSourceMapMiddleware: function() {
        return getSourceMapMiddleware;
    }
});
const _interop_require_default = require("@swc/helpers/_/_interop_require_default");
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _shared = require("./shared");
const _promises = /*#__PURE__*/ _interop_require_wildcard._(require("fs/promises"));
const _path = /*#__PURE__*/ _interop_require_default._(require("path"));
const _url = /*#__PURE__*/ _interop_require_default._(require("url"));
const _launchEditor = require("../internal/helpers/launchEditor");
const _sourcemap08 = require("next/dist/compiled/source-map08");
const _getsourcemapfromfile = require("../internal/helpers/get-source-map-from-file");
const _nodemodule = require("node:module");
function shouldIgnorePath(modulePath) {
    return modulePath.includes('node_modules') || // Only relevant for when Next.js is symlinked e.g. in the Next.js monorepo
    modulePath.includes('next/dist');
}
const currentSourcesByFile = new Map();
async function batchedTraceSource(project, frame) {
    const file = frame.file ? decodeURIComponent(frame.file) : undefined;
    if (!file) return;
    const sourceFrame = await project.traceSource(frame);
    if (!sourceFrame) {
        var _frame_line, _frame_column, _frame_methodName;
        return {
            frame: {
                file,
                lineNumber: (_frame_line = frame.line) != null ? _frame_line : 0,
                column: (_frame_column = frame.column) != null ? _frame_column : 0,
                methodName: (_frame_methodName = frame.methodName) != null ? _frame_methodName : '<unknown>',
                ignored: shouldIgnorePath(frame.file),
                arguments: []
            },
            source: null
        };
    }
    let source = null;
    // Don't look up source for node_modules or internals. These can often be large bundled files.
    const ignored = shouldIgnorePath(sourceFrame.file) || // isInternal means resource starts with turbopack://[turbopack]
    !!sourceFrame.isInternal;
    if (sourceFrame && sourceFrame.file && !ignored) {
        let sourcePromise = currentSourcesByFile.get(sourceFrame.file);
        if (!sourcePromise) {
            sourcePromise = project.getSourceForAsset(sourceFrame.file);
            currentSourcesByFile.set(sourceFrame.file, sourcePromise);
            setTimeout(()=>{
                // Cache file reads for 100ms, as frames will often reference the same
                // files and can be large.
                currentSourcesByFile.delete(sourceFrame.file);
            }, 100);
        }
        source = await sourcePromise;
    }
    var _sourceFrame_line, _sourceFrame_column, _sourceFrame_methodName, _ref;
    // TODO: get ignoredList from turbopack source map
    const ignorableFrame = {
        file: sourceFrame.file,
        lineNumber: (_sourceFrame_line = sourceFrame.line) != null ? _sourceFrame_line : 0,
        column: (_sourceFrame_column = sourceFrame.column) != null ? _sourceFrame_column : 0,
        methodName: (_ref = (_sourceFrame_methodName = sourceFrame.methodName) != null ? _sourceFrame_methodName : frame.methodName) != null ? _ref : '<unknown>',
        ignored,
        arguments: []
    };
    return {
        frame: ignorableFrame,
        source
    };
}
function createStackFrame(searchParams) {
    const fileParam = searchParams.get('file');
    if (!fileParam) {
        return undefined;
    }
    // rsc://React/Server/file://<filename>?42 => file://<filename>
    const file = fileParam.replace(/^rsc:\/\/React\/[^/]+\//, '').replace(/\?\d+$/, '');
    var _searchParams_get, _searchParams_get1, _searchParams_get2;
    return {
        file,
        methodName: (_searchParams_get = searchParams.get('methodName')) != null ? _searchParams_get : '<unknown>',
        line: parseInt((_searchParams_get1 = searchParams.get('lineNumber')) != null ? _searchParams_get1 : '0', 10) || 0,
        column: parseInt((_searchParams_get2 = searchParams.get('column')) != null ? _searchParams_get2 : '0', 10) || 0,
        isServer: searchParams.get('isServer') === 'true'
    };
}
/**
 * Finds the sourcemap payload applicable to a given frame.
 * Equal to the input unless an Index Source Map is used.
 */ function findApplicableSourceMapPayload(frame, payload) {
    if ('sections' in payload) {
        var _frame_line;
        const frameLine = (_frame_line = frame.line) != null ? _frame_line : 0;
        var _frame_column;
        const frameColumn = (_frame_column = frame.column) != null ? _frame_column : 0;
        // Sections must not overlap and must be sorted: https://tc39.es/source-map/#section-object
        // Therefore the last section that has an offset less than or equal to the frame is the applicable one.
        // TODO(veil): Binary search
        let section = payload.sections[0];
        for(let i = 0; i < payload.sections.length && payload.sections[i].offset.line <= frameLine && payload.sections[i].offset.column <= frameColumn; i++){
            section = payload.sections[i];
        }
        return section === undefined ? undefined : section.map;
    } else {
        return payload;
    }
}
async function nativeTraceSource(frame) {
    const sourceMap = (0, _nodemodule.findSourceMap)(// TODO(veil): Why are the frames sent encoded?
    decodeURIComponent(frame.file));
    if (sourceMap !== undefined) {
        const traced = await _sourcemap08.SourceMapConsumer.with(sourceMap.payload, null, async (consumer)=>{
            var _frame_line, _frame_column;
            const originalPosition = consumer.originalPositionFor({
                line: (_frame_line = frame.line) != null ? _frame_line : 1,
                column: (_frame_column = frame.column) != null ? _frame_column : 1
            });
            if (originalPosition.source === null) {
                return null;
            }
            var _consumer_sourceContentFor;
            const sourceContent = (_consumer_sourceContentFor = consumer.sourceContentFor(originalPosition.source, /* returnNullOnMissing */ true)) != null ? _consumer_sourceContentFor : null;
            return {
                originalPosition,
                sourceContent
            };
        });
        if (traced !== null) {
            var // default is not a valid identifier in JS so webpack uses a custom variable when it's an unnamed default export
            // Resolve it back to `default` for the method name if the source position didn't have the method.
            _frame_methodName_replace, _frame_methodName, _originalPosition_source;
            const { originalPosition, sourceContent } = traced;
            const applicableSourceMap = findApplicableSourceMapPayload(frame, sourceMap.payload);
            // TODO(veil): Upstream a method to sourcemap consumer that immediately says if a frame is ignored or not.
            let ignored = false;
            if (applicableSourceMap === undefined) {
                console.error('No applicable source map found in sections for frame', frame);
            } else {
                var _applicableSourceMap_ignoreList;
                // TODO: O(n^2). Consider moving `ignoreList` into a Set
                const sourceIndex = applicableSourceMap.sources.indexOf(originalPosition.source);
                var _applicableSourceMap_ignoreList_includes;
                ignored = (_applicableSourceMap_ignoreList_includes = (_applicableSourceMap_ignoreList = applicableSourceMap.ignoreList) == null ? void 0 : _applicableSourceMap_ignoreList.includes(sourceIndex)) != null ? _applicableSourceMap_ignoreList_includes : false;
            }
            var _originalPosition_column, _originalPosition_line;
            const originalStackFrame = {
                methodName: originalPosition.name || ((_frame_methodName = frame.methodName) == null ? void 0 : (_frame_methodName_replace = _frame_methodName.replace('__WEBPACK_DEFAULT_EXPORT__', 'default')) == null ? void 0 : _frame_methodName_replace.replace('__webpack_exports__.', '')) || '<unknown>',
                column: ((_originalPosition_column = originalPosition.column) != null ? _originalPosition_column : 0) + 1,
                file: ((_originalPosition_source = originalPosition.source) == null ? void 0 : _originalPosition_source.startsWith('file://')) ? _path.default.relative(process.cwd(), _url.default.fileURLToPath(originalPosition.source)) : originalPosition.source,
                lineNumber: (_originalPosition_line = originalPosition.line) != null ? _originalPosition_line : 0,
                // TODO: c&p from async createOriginalStackFrame but why not frame.arguments?
                arguments: [],
                ignored
            };
            return {
                frame: originalStackFrame,
                source: sourceContent
            };
        }
    }
    return undefined;
}
async function createOriginalStackFrame(project, frame) {
    var _ref;
    const traced = (_ref = await nativeTraceSource(frame)) != null ? _ref : // TODO(veil): When would the bundler know more than native?
    // If it's faster, try the bundler first and fall back to native later.
    await batchedTraceSource(project, frame);
    if (!traced) {
        return null;
    }
    return {
        originalStackFrame: traced.frame,
        originalCodeFrame: (0, _shared.getOriginalCodeFrame)(traced.frame, traced.source)
    };
}
function getOverlayMiddleware(project) {
    return async function(req, res, next) {
        const { pathname, searchParams } = new URL(req.url, 'http://n');
        if (pathname === '/__nextjs_original-stack-frame') {
            const frame = createStackFrame(searchParams);
            if (!frame) return (0, _shared.badRequest)(res);
            let originalStackFrame;
            try {
                originalStackFrame = await createOriginalStackFrame(project, frame);
            } catch (e) {
                return (0, _shared.internalServerError)(res, e.message);
            }
            if (!originalStackFrame) {
                res.statusCode = 404;
                res.end('Unable to resolve sourcemap');
                return;
            }
            return (0, _shared.json)(res, originalStackFrame);
        } else if (pathname === '/__nextjs_launch-editor') {
            const frame = createStackFrame(searchParams);
            if (!frame) return (0, _shared.badRequest)(res);
            const fileExists = await _promises.default.access(frame.file, _promises.constants.F_OK).then(()=>true, ()=>false);
            if (!fileExists) return (0, _shared.noContent)(res);
            try {
                var _frame_line, _frame_column;
                (0, _launchEditor.launchEditor)(frame.file, (_frame_line = frame.line) != null ? _frame_line : 1, (_frame_column = frame.column) != null ? _frame_column : 1);
            } catch (err) {
                console.log('Failed to launch editor:', err);
                return (0, _shared.internalServerError)(res);
            }
            (0, _shared.noContent)(res);
        }
        return next();
    };
}
function getSourceMapMiddleware(project) {
    return async function(req, res, next) {
        const { pathname, searchParams } = new URL(req.url, 'http://n');
        if (pathname !== '/__nextjs_source-map') {
            return next();
        }
        let filename = searchParams.get('filename');
        if (!filename) {
            return (0, _shared.badRequest)(res);
        }
        // TODO(veil): Always try the native version first.
        // Externals could also be files that aren't bundled via Webpack.
        if (filename.startsWith('webpack://') || filename.startsWith('webpack-internal:///')) {
            const sourceMap = (0, _nodemodule.findSourceMap)(filename);
            if (sourceMap) {
                return (0, _shared.json)(res, sourceMap.payload);
            }
            return (0, _shared.noContent)(res);
        }
        try {
            // Turbopack chunk filenames might be URL-encoded.
            filename = decodeURI(filename);
            if (_path.default.isAbsolute(filename)) {
                filename = _url.default.pathToFileURL(filename).href;
            }
            const sourceMapString = await project.getSourceMap(filename);
            if (sourceMapString) {
                return (0, _shared.jsonString)(res, sourceMapString);
            }
            if (filename.startsWith('file:')) {
                const sourceMap = await (0, _getsourcemapfromfile.getSourceMapFromFile)(filename);
                if (sourceMap) {
                    return (0, _shared.json)(res, sourceMap);
                }
            }
        } catch (error) {
            console.error('Failed to get source map:', error);
        }
        (0, _shared.noContent)(res);
    };
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=middleware-turbopack.js.map