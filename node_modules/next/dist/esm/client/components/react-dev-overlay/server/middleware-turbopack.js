import { badRequest, findSourcePackage, getOriginalCodeFrame, internalServerError, json, noContent } from "./shared";
import fs, { constants as FS } from "fs/promises";
import { launchEditor } from "../internal/helpers/launchEditor";
const currentSourcesByFile = new Map();
export async function batchedTraceSource(project, frame) {
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
export async function createOriginalStackFrame(project, frame) {
    const traced = await batchedTraceSource(project, frame);
    if (!traced) {
        const sourcePackage = findSourcePackage(frame);
        if (sourcePackage) return {
            sourcePackage
        };
        return null;
    }
    return {
        originalStackFrame: traced.frame,
        originalCodeFrame: getOriginalCodeFrame(traced.frame, traced.source),
        sourcePackage: findSourcePackage(traced.frame)
    };
}
export function getOverlayMiddleware(project) {
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
                return internalServerError(res, e.message);
            }
            if (!originalStackFrame) {
                res.statusCode = 404;
                return res.end("Unable to resolve sourcemap");
            }
            return json(res, originalStackFrame);
        } else if (pathname === "/__nextjs_launch-editor") {
            if (!frame.file) return badRequest(res);
            const fileExists = await fs.access(frame.file, FS.F_OK).then(()=>true, ()=>false);
            if (!fileExists) return noContent(res);
            try {
                var _frame_line, _frame_column;
                launchEditor(frame.file, (_frame_line = frame.line) != null ? _frame_line : 1, (_frame_column = frame.column) != null ? _frame_column : 1);
            } catch (err) {
                console.log("Failed to launch editor:", err);
                return internalServerError(res);
            }
            noContent(res);
        }
    };
}

//# sourceMappingURL=middleware-turbopack.js.map