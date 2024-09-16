import { constants as FS, promises as fs } from "fs";
import path from "path";
import { SourceMapConsumer } from "next/dist/compiled/source-map08";
import { getRawSourceMap } from "../internal/helpers/getRawSourceMap";
import { launchEditor } from "../internal/helpers/launchEditor";
import { badRequest, findSourcePackage, getOriginalCodeFrame, internalServerError, json, noContent } from "./shared";
export { getServerError } from "../internal/helpers/nodeStackFrames";
export { parseStack } from "../internal/helpers/parseStack";
function getModuleId(compilation, module) {
    return compilation.chunkGraph.getModuleId(module);
}
function getModuleById(id, compilation) {
    return [
        ...compilation.modules
    ].find((searchModule)=>getModuleId(compilation, searchModule) === id);
}
function findModuleNotFoundFromError(errorMessage) {
    var _errorMessage_match;
    return errorMessage == null ? void 0 : (_errorMessage_match = errorMessage.match(/'([^']+)' module/)) == null ? void 0 : _errorMessage_match[1];
}
function getModuleSource(compilation, module) {
    var _compilation_codeGenerationResults_get;
    if (!module) return null;
    var _compilation_codeGenerationResults_get_sources_get;
    return (_compilation_codeGenerationResults_get_sources_get = (_compilation_codeGenerationResults_get = compilation.codeGenerationResults.get(module)) == null ? void 0 : _compilation_codeGenerationResults_get.sources.get("javascript")) != null ? _compilation_codeGenerationResults_get_sources_get : null;
}
function getSourcePath(source) {
    return source.replace(/^(webpack:\/\/\/|webpack:\/\/|webpack:\/\/_N_E\/)/, "");
}
async function findOriginalSourcePositionAndContent(webpackSource, position) {
    const consumer = await new SourceMapConsumer(webpackSource.map());
    try {
        var _position_column;
        const sourcePosition = consumer.originalPositionFor({
            line: position.line,
            column: (_position_column = position.column) != null ? _position_column : 0
        });
        if (!sourcePosition.source) {
            return null;
        }
        var _consumer_sourceContentFor;
        const sourceContent = (_consumer_sourceContentFor = consumer.sourceContentFor(sourcePosition.source, /* returnNullOnMissing */ true)) != null ? _consumer_sourceContentFor : null;
        return {
            sourcePosition,
            sourceContent
        };
    } finally{
        consumer.destroy();
    }
}
function findOriginalSourcePositionAndContentFromCompilation(moduleId, importedModule, compilation) {
    var _module_buildInfo_importLocByPath, _module_buildInfo;
    const module = getModuleById(moduleId, compilation);
    var _module_buildInfo_importLocByPath_get;
    return (_module_buildInfo_importLocByPath_get = module == null ? void 0 : (_module_buildInfo = module.buildInfo) == null ? void 0 : (_module_buildInfo_importLocByPath = _module_buildInfo.importLocByPath) == null ? void 0 : _module_buildInfo_importLocByPath.get(importedModule)) != null ? _module_buildInfo_importLocByPath_get : null;
}
export async function createOriginalStackFrame(param) {
    let { source, moduleId, modulePath, rootDirectory, frame, errorMessage, compilation } = param;
    var // default is not a valid identifier in JS so webpack uses a custom variable when it's an unnamed default export
    // Resolve it back to `default` for the method name if the source position didn't have the method.
    _frame_methodName_replace, _frame_methodName;
    const { lineNumber, column } = frame;
    const moduleNotFound = findModuleNotFoundFromError(errorMessage);
    const result = await (async ()=>{
        if (moduleNotFound) {
            if (!compilation) return null;
            return findOriginalSourcePositionAndContentFromCompilation(moduleId, moduleNotFound, compilation);
        }
        // This returns 1-based lines and 0-based columns
        return await findOriginalSourcePositionAndContent(source, {
            line: lineNumber != null ? lineNumber : 1,
            column
        });
    })();
    if (!(result == null ? void 0 : result.sourcePosition.source)) return null;
    const { sourcePosition, sourceContent } = result;
    const filePath = path.resolve(rootDirectory, getSourcePath(// When sourcePosition.source is the loader path the modulePath is generally better.
    (sourcePosition.source.includes("|") ? modulePath : sourcePosition.source) || modulePath));
    var _sourcePosition_column;
    const traced = {
        file: sourceContent ? path.relative(rootDirectory, filePath) : sourcePosition.source,
        lineNumber: sourcePosition.line,
        column: ((_sourcePosition_column = sourcePosition.column) != null ? _sourcePosition_column : 0) + 1,
        methodName: sourcePosition.name || ((_frame_methodName = frame.methodName) == null ? void 0 : (_frame_methodName_replace = _frame_methodName.replace("__WEBPACK_DEFAULT_EXPORT__", "default")) == null ? void 0 : _frame_methodName_replace.replace("__webpack_exports__.", "")),
        arguments: []
    };
    return {
        originalStackFrame: traced,
        originalCodeFrame: getOriginalCodeFrame(traced, sourceContent),
        sourcePackage: findSourcePackage(traced)
    };
}
export async function getSourceById(isFile, id, compilation) {
    if (isFile) {
        const fileContent = await fs.readFile(id, "utf-8").catch(()=>null);
        if (fileContent == null) {
            return null;
        }
        const map = getRawSourceMap(fileContent);
        if (map == null) {
            return null;
        }
        return {
            map () {
                return map;
            }
        };
    }
    try {
        if (!compilation) {
            return null;
        }
        const module = getModuleById(id, compilation);
        const moduleSource = getModuleSource(compilation, module);
        return moduleSource;
    } catch (err) {
        console.error('Failed to lookup module by ID ("' + id + '"):', err);
        return null;
    }
}
export function getOverlayMiddleware(options) {
    return async function(req, res, next) {
        const { pathname, searchParams } = new URL("http://n" + req.url);
        var _searchParams_get, _searchParams_get1;
        const frame = {
            file: searchParams.get("file"),
            methodName: searchParams.get("methodName"),
            lineNumber: parseInt((_searchParams_get = searchParams.get("lineNumber")) != null ? _searchParams_get : "0", 10) || 0,
            column: parseInt((_searchParams_get1 = searchParams.get("column")) != null ? _searchParams_get1 : "0", 10) || 0,
            arguments: searchParams.getAll("arguments").filter(Boolean)
        };
        const isServer = searchParams.get("isServer") === "true";
        const isEdgeServer = searchParams.get("isEdgeServer") === "true";
        const isAppDirectory = searchParams.get("isAppDirectory") === "true";
        if (pathname === "/__nextjs_original-stack-frame") {
            const isClient = !isServer && !isEdgeServer;
            let sourcePackage = findSourcePackage(frame);
            if (!(/^(webpack-internal:\/\/\/|(file|webpack):\/\/)/.test(frame.file) && frame.lineNumber)) {
                if (sourcePackage) return json(res, {
                    sourcePackage
                });
                return badRequest(res);
            }
            const moduleId = frame.file.replace(/^(webpack-internal:\/\/\/|file:\/\/|webpack:\/\/(_N_E\/)?)/, "");
            const modulePath = frame.file.replace(/^(webpack-internal:\/\/\/|file:\/\/|webpack:\/\/(_N_E\/)?)(\(.*\)\/?)/, "");
            let source = null;
            let compilation;
            const isFile = frame.file.startsWith("file:");
            try {
                if (isClient || isAppDirectory) {
                    var _options_stats;
                    compilation = (_options_stats = options.stats()) == null ? void 0 : _options_stats.compilation;
                    // Try Client Compilation first
                    // In `pages` we leverage `isClientError` to check
                    // In `app` it depends on if it's a server / client component and when the code throws. E.g. during HTML rendering it's the server/edge compilation.
                    source = await getSourceById(isFile, moduleId, compilation);
                }
                // Try Server Compilation
                // In `pages` this could be something imported in getServerSideProps/getStaticProps as the code for those is tree-shaken.
                // In `app` this finds server components and code that was imported from a server component. It also covers when client component code throws during HTML rendering.
                if ((isServer || isAppDirectory) && source === null) {
                    var _options_serverStats;
                    compilation = (_options_serverStats = options.serverStats()) == null ? void 0 : _options_serverStats.compilation;
                    source = await getSourceById(isFile, moduleId, compilation);
                }
                // Try Edge Server Compilation
                // Both cases are the same as Server Compilation, main difference is that it covers `runtime: 'edge'` pages/app routes.
                if ((isEdgeServer || isAppDirectory) && source === null) {
                    var _options_edgeServerStats;
                    compilation = (_options_edgeServerStats = options.edgeServerStats()) == null ? void 0 : _options_edgeServerStats.compilation;
                    source = await getSourceById(isFile, moduleId, compilation);
                }
            } catch (err) {
                console.log("Failed to get source map:", err);
                return internalServerError(res);
            }
            if (!source) {
                if (sourcePackage) return json(res, {
                    sourcePackage
                });
                return noContent(res);
            }
            try {
                const originalStackFrameResponse = await createOriginalStackFrame({
                    frame,
                    source,
                    moduleId,
                    modulePath,
                    rootDirectory: options.rootDirectory,
                    compilation
                });
                if (originalStackFrameResponse === null) {
                    if (sourcePackage) return json(res, {
                        sourcePackage
                    });
                    return noContent(res);
                }
                return json(res, originalStackFrameResponse);
            } catch (err) {
                console.log("Failed to parse source map:", err);
                return internalServerError(res);
            }
        } else if (pathname === "/__nextjs_launch-editor") {
            if (!frame.file) return badRequest(res);
            // frame files may start with their webpack layer, like (middleware)/middleware.js
            const filePath = path.resolve(options.rootDirectory, frame.file.replace(/^\([^)]+\)\//, ""));
            const fileExists = await fs.access(filePath, FS.F_OK).then(()=>true, ()=>false);
            if (!fileExists) return noContent(res);
            try {
                var _frame_column;
                await launchEditor(filePath, frame.lineNumber, (_frame_column = frame.column) != null ? _frame_column : 1);
            } catch (err) {
                console.log("Failed to launch editor:", err);
                return internalServerError(res);
            }
            return noContent(res);
        }
        return next();
    };
}

//# sourceMappingURL=middleware.js.map