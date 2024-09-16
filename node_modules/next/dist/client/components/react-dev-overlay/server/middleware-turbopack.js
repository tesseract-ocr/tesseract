"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    batchedTraceSource: null,
    createOriginalStackFrame: null,
    getOverlayMiddleware: null
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
    createOriginalStackFrame: function() {
        return createOriginalStackFrame;
    },
    getOverlayMiddleware: function() {
        return getOverlayMiddleware;
    }
});
const _interop_require_wildcard = require("@swc/helpers/_/_interop_require_wildcard");
const _shared = require("./shared");
const _promises = /*#__PURE__*/ _interop_require_wildcard._(require("fs/promises"));
const _launchEditor = require("../internal/helpers/launchEditor");
const currentSourcesByFile = new Map();
async function batchedTraceSource(project, frame) {
    const file = frame.file ? decodeURIComponent(frame.file) : undefined;
    if (!file) return;
    const sourceFrame = await project.traceSource(frame);
    if (!sourceFrame) return;
    let source = null;
    // Don't look up source for node_modules or internals. These can often be large bundled files.
    if (sourceFrame.file && !(sourceFrame.file.includes("node_modules") || sourceFrame.isInternal)) {
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
    return {
        frame: {
            file: sourceFrame.file,
            lineNumber: (_sourceFrame_line = sourceFrame.line) != null ? _sourceFrame_line : 0,
            column: (_sourceFrame_column = sourceFrame.column) != null ? _sourceFrame_column : 0,
            methodName: (_ref = (_sourceFrame_methodName = sourceFrame.methodName) != null ? _sourceFrame_methodName : frame.methodName) != null ? _ref : "<unknown>",
            arguments: []
        },
        source
    };
}
async function createOriginalStackFrame(project, frame) {
    const traced = await batchedTraceSource(project, frame);
    if (!traced) {
        const sourcePackage = (0, _shared.findSourcePackage)(frame);
        if (sourcePackage) return {
            sourcePackage
        };
        return null;
    }
    return {
        originalStackFrame: traced.frame,
        originalCodeFrame: (0, _shared.getOriginalCodeFrame)(traced.frame, traced.source),
        sourcePackage: (0, _shared.findSourcePackage)(traced.frame)
    };
}
function getOverlayMiddleware(project) {
    return async function(req, res) {
        const { pathname, searchParams } = new URL(req.url, "http://n");
        var _searchParams_get, _searchParams_get1, _searchParams_get2;
        const frame = {
            file: searchParams.get("file"),
            methodName: (_searchParams_get = searchParams.get("methodName")) != null ? _searchParams_get : "<unknown>",
            line: parseInt((_searchParams_get1 = searchParams.get("lineNumber")) != null ? _searchParams_get1 : "0", 10) || 0,
            column: parseInt((_searchParams_get2 = searchParams.get("column")) != null ? _searchParams_get2 : "0", 10) || 0,
            isServer: searchParams.get("isServer") === "true"
        };
        if (pathname === "/__nextjs_original-stack-frame") {
            let originalStackFrame;
            try {
                originalStackFrame = await createOriginalStackFrame(project, frame);
            } catch (e) {
                return (0, _shared.internalServerError)(res, e.message);
            }
            if (!originalStackFrame) {
                res.statusCode = 404;
                return res.end("Unable to resolve sourcemap");
            }
            return (0, _shared.json)(res, originalStackFrame);
        } else if (pathname === "/__nextjs_launch-editor") {
            if (!frame.file) return (0, _shared.badRequest)(res);
            const fileExists = await _promises.default.access(frame.file, _promises.constants.F_OK).then(()=>true, ()=>false);
            if (!fileExists) return (0, _shared.noContent)(res);
            try {
                var _frame_line, _frame_column;
                (0, _launchEditor.launchEditor)(frame.file, (_frame_line = frame.line) != null ? _frame_line : 1, (_frame_column = frame.column) != null ? _frame_column : 1);
            } catch (err) {
                console.log("Failed to launch editor:", err);
                return (0, _shared.internalServerError)(res);
            }
            (0, _shared.noContent)(res);
        }
    };
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=middleware-turbopack.js.map