/**
 * This is a TypeScript language service plugin for Next.js app directory,
 * it provides the following features:
 *
 * - Warns about disallowed React APIs in server components.
 * - Warns about disallowed layout and page exports.
 * - Autocompletion for entry configurations.
 * - Hover hint and docs for entry configurations.
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createTSPlugin", {
    enumerable: true,
    get: function() {
        return createTSPlugin;
    }
});
const _utils = require("./utils");
const _constant = require("./constant");
const _config = /*#__PURE__*/ _interop_require_default(require("./rules/config"));
const _server = /*#__PURE__*/ _interop_require_default(require("./rules/server"));
const _entry = /*#__PURE__*/ _interop_require_default(require("./rules/entry"));
const _clientboundary = /*#__PURE__*/ _interop_require_default(require("./rules/client-boundary"));
const _serverboundary = /*#__PURE__*/ _interop_require_default(require("./rules/server-boundary"));
const _metadata = /*#__PURE__*/ _interop_require_default(require("./rules/metadata"));
const _error = /*#__PURE__*/ _interop_require_default(require("./rules/error"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const createTSPlugin = ({ typescript: ts })=>{
    function create(info) {
        (0, _utils.init)({
            ts,
            info
        });
        // Set up decorator object
        const proxy = Object.create(null);
        for (let k of Object.keys(info.languageService)){
            const x = info.languageService[k];
            proxy[k] = (...args)=>x.apply(info.languageService, args);
        }
        // Auto completion
        proxy.getCompletionsAtPosition = (fileName, position, options)=>{
            let prior = info.languageService.getCompletionsAtPosition(fileName, position, options) || {
                isGlobalCompletion: false,
                isMemberCompletion: false,
                isNewIdentifierLocation: false,
                entries: []
            };
            if (!(0, _utils.isAppEntryFile)(fileName)) return prior;
            // If it's a server entry.
            const entryInfo = (0, _utils.getEntryInfo)(fileName);
            if (!entryInfo.client) {
                // Remove specified entries from completion list
                prior.entries = _server.default.filterCompletionsAtPosition(prior.entries);
                // Provide autocompletion for metadata fields
                prior = _metadata.default.filterCompletionsAtPosition(fileName, position, options, prior);
            }
            // Add auto completions for export configs.
            _config.default.addCompletionsAtPosition(fileName, position, prior);
            const source = (0, _utils.getSource)(fileName);
            if (!source) return prior;
            ts.forEachChild(source, (node)=>{
                // Auto completion for default export function's props.
                if ((0, _utils.isPositionInsideNode)(position, node) && (0, _utils.isDefaultFunctionExport)(node)) {
                    prior.entries.push(..._entry.default.getCompletionsAtPosition(fileName, node, position));
                }
            });
            return prior;
        };
        // Show auto completion details
        proxy.getCompletionEntryDetails = (fileName, position, entryName, formatOptions, source, preferences, data)=>{
            const entryCompletionEntryDetails = _config.default.getCompletionEntryDetails(entryName, data);
            if (entryCompletionEntryDetails) return entryCompletionEntryDetails;
            const metadataCompletionEntryDetails = _metadata.default.getCompletionEntryDetails(fileName, position, entryName, formatOptions, source, preferences, data);
            if (metadataCompletionEntryDetails) return metadataCompletionEntryDetails;
            return info.languageService.getCompletionEntryDetails(fileName, position, entryName, formatOptions, source, preferences, data);
        };
        // Quick info
        proxy.getQuickInfoAtPosition = (fileName, position)=>{
            const prior = info.languageService.getQuickInfoAtPosition(fileName, position);
            if (!(0, _utils.isAppEntryFile)(fileName)) return prior;
            // Remove type suggestions for disallowed APIs in server components.
            const entryInfo = (0, _utils.getEntryInfo)(fileName);
            if (!entryInfo.client) {
                const definitions = info.languageService.getDefinitionAtPosition(fileName, position);
                if (definitions && _server.default.hasDisallowedReactAPIDefinition(definitions)) {
                    return;
                }
                const metadataInfo = _metadata.default.getQuickInfoAtPosition(fileName, position);
                if (metadataInfo) return metadataInfo;
            }
            const overridden = _config.default.getQuickInfoAtPosition(fileName, position);
            if (overridden) return overridden;
            return prior;
        };
        // Show errors for disallowed imports
        proxy.getSemanticDiagnostics = (fileName)=>{
            const prior = info.languageService.getSemanticDiagnostics(fileName);
            const source = (0, _utils.getSource)(fileName);
            if (!source) return prior;
            let isClientEntry = false;
            let isServerEntry = false;
            const isAppEntry = (0, _utils.isAppEntryFile)(fileName);
            try {
                const entryInfo = (0, _utils.getEntryInfo)(fileName, true);
                isClientEntry = entryInfo.client;
                isServerEntry = entryInfo.server;
            } catch (e) {
                prior.push({
                    file: source,
                    category: ts.DiagnosticCategory.Error,
                    code: _constant.NEXT_TS_ERRORS.MISPLACED_ENTRY_DIRECTIVE,
                    ...e
                });
                isClientEntry = false;
                isServerEntry = false;
            }
            if ((0, _utils.isInsideApp)(fileName)) {
                const errorDiagnostic = _error.default.getSemanticDiagnostics(source, isClientEntry);
                prior.push(...errorDiagnostic);
            }
            ts.forEachChild(source, (node)=>{
                var _node_modifiers, _node_modifiers1;
                if (ts.isImportDeclaration(node)) {
                    // import ...
                    if (isAppEntry) {
                        if (!isClientEntry || isServerEntry) {
                            // Check if it has valid imports in the server layer
                            const diagnostics = _server.default.getSemanticDiagnosticsForImportDeclaration(source, node);
                            prior.push(...diagnostics);
                        }
                    }
                } else if (ts.isVariableStatement(node) && ((_node_modifiers = node.modifiers) == null ? void 0 : _node_modifiers.some((m)=>m.kind === ts.SyntaxKind.ExportKeyword))) {
                    // export const ...
                    if (isAppEntry) {
                        // Check if it has correct option exports
                        const diagnostics = _config.default.getSemanticDiagnosticsForExportVariableStatement(source, node);
                        const metadataDiagnostics = isClientEntry ? _metadata.default.getSemanticDiagnosticsForExportVariableStatementInClientEntry(fileName, node) : _metadata.default.getSemanticDiagnosticsForExportVariableStatement(fileName, node);
                        prior.push(...diagnostics, ...metadataDiagnostics);
                    }
                    if (isClientEntry) {
                        prior.push(..._clientboundary.default.getSemanticDiagnosticsForExportVariableStatement(source, node));
                    }
                    if (isServerEntry) {
                        prior.push(..._serverboundary.default.getSemanticDiagnosticsForExportVariableStatement(source, node));
                    }
                } else if ((0, _utils.isDefaultFunctionExport)(node)) {
                    // export default function ...
                    if (isAppEntry) {
                        const diagnostics = _entry.default.getSemanticDiagnostics(fileName, source, node);
                        prior.push(...diagnostics);
                    }
                    if (isClientEntry) {
                        prior.push(..._clientboundary.default.getSemanticDiagnosticsForFunctionExport(source, node));
                    }
                    if (isServerEntry) {
                        prior.push(..._serverboundary.default.getSemanticDiagnosticsForFunctionExport(source, node));
                    }
                } else if (ts.isFunctionDeclaration(node) && ((_node_modifiers1 = node.modifiers) == null ? void 0 : _node_modifiers1.some((m)=>m.kind === ts.SyntaxKind.ExportKeyword))) {
                    // export function ...
                    if (isAppEntry) {
                        const metadataDiagnostics = isClientEntry ? _metadata.default.getSemanticDiagnosticsForExportVariableStatementInClientEntry(fileName, node) : _metadata.default.getSemanticDiagnosticsForExportVariableStatement(fileName, node);
                        prior.push(...metadataDiagnostics);
                    }
                    if (isClientEntry) {
                        prior.push(..._clientboundary.default.getSemanticDiagnosticsForFunctionExport(source, node));
                    }
                    if (isServerEntry) {
                        prior.push(..._serverboundary.default.getSemanticDiagnosticsForFunctionExport(source, node));
                    }
                } else if (ts.isExportDeclaration(node)) {
                    // export { ... }
                    if (isAppEntry) {
                        const metadataDiagnostics = isClientEntry ? _metadata.default.getSemanticDiagnosticsForExportDeclarationInClientEntry(fileName, node) : _metadata.default.getSemanticDiagnosticsForExportDeclaration(fileName, node);
                        prior.push(...metadataDiagnostics);
                    }
                    if (isServerEntry) {
                        prior.push(..._serverboundary.default.getSemanticDiagnosticsForExportDeclaration(source, node));
                    }
                }
            });
            return prior;
        };
        // Get definition and link for specific node
        proxy.getDefinitionAndBoundSpan = (fileName, position)=>{
            const entryInfo = (0, _utils.getEntryInfo)(fileName);
            if ((0, _utils.isAppEntryFile)(fileName) && !entryInfo.client) {
                const metadataDefinition = _metadata.default.getDefinitionAndBoundSpan(fileName, position);
                if (metadataDefinition) return metadataDefinition;
            }
            return info.languageService.getDefinitionAndBoundSpan(fileName, position);
        };
        return proxy;
    }
    return {
        create
    };
};

//# sourceMappingURL=index.js.map