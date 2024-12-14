import { types as BabelTypes } from 'next/dist/compiled/babel/core';
import { STRING_LITERAL_DROP_BUNDLE } from '../../../shared/lib/constants';
const CONFIG_KEY = 'config';
// replace program path with just a variable with the drop identifier
function replaceBundle(path, t) {
    path.parentPath.replaceWith(t.program([
        t.variableDeclaration('const', [
            t.variableDeclarator(t.identifier(STRING_LITERAL_DROP_BUNDLE), t.stringLiteral(`${STRING_LITERAL_DROP_BUNDLE} ${Date.now()}`))
        ])
    ], []));
}
function errorMessage(state, details) {
    const pageName = (state.filename || '').split(state.cwd || '').pop() || 'unknown';
    return `Invalid page config export found. ${details} in file ${pageName}. See: https://nextjs.org/docs/messages/invalid-page-config`;
}
// config to parsing pageConfig for client bundles
export default function nextPageConfig({ types: t }) {
    return {
        visitor: {
            Program: {
                enter (path, state) {
                    path.traverse({
                        ExportDeclaration (exportPath, exportState) {
                            var _exportPath_node_specifiers;
                            if (BabelTypes.isExportNamedDeclaration(exportPath.node) && ((_exportPath_node_specifiers = exportPath.node.specifiers) == null ? void 0 : _exportPath_node_specifiers.some((specifier)=>{
                                return (t.isIdentifier(specifier.exported) ? specifier.exported.name : specifier.exported.value) === CONFIG_KEY;
                            })) && BabelTypes.isStringLiteral(exportPath.node.source)) {
                                throw new Error(errorMessage(exportState, 'Expected object but got export from'));
                            }
                        },
                        ExportNamedDeclaration (exportPath, exportState) {
                            var _exportPath_node_declaration, _exportPath_scope_getBinding;
                            if (exportState.bundleDropped || !exportPath.node.declaration && exportPath.node.specifiers.length === 0) {
                                return;
                            }
                            const config = {};
                            const declarations = [
                                ...((_exportPath_node_declaration = exportPath.node.declaration) == null ? void 0 : _exportPath_node_declaration.declarations) || [],
                                (_exportPath_scope_getBinding = exportPath.scope.getBinding(CONFIG_KEY)) == null ? void 0 : _exportPath_scope_getBinding.path.node
                            ].filter(Boolean);
                            for (const specifier of exportPath.node.specifiers){
                                if ((t.isIdentifier(specifier.exported) ? specifier.exported.name : specifier.exported.value) === CONFIG_KEY) {
                                    // export {} from 'somewhere'
                                    if (BabelTypes.isStringLiteral(exportPath.node.source)) {
                                        throw new Error(errorMessage(exportState, `Expected object but got import`));
                                    // import hello from 'world'
                                    // export { hello as config }
                                    } else if (BabelTypes.isIdentifier(specifier.local)) {
                                        var _exportPath_scope_getBinding1;
                                        if (BabelTypes.isImportSpecifier((_exportPath_scope_getBinding1 = exportPath.scope.getBinding(specifier.local.name)) == null ? void 0 : _exportPath_scope_getBinding1.path.node)) {
                                            throw new Error(errorMessage(exportState, `Expected object but got import`));
                                        }
                                    }
                                }
                            }
                            for (const declaration of declarations){
                                if (!BabelTypes.isIdentifier(declaration.id, {
                                    name: CONFIG_KEY
                                })) {
                                    continue;
                                }
                                let { init } = declaration;
                                if (BabelTypes.isTSAsExpression(init)) {
                                    init = init.expression;
                                }
                                if (!BabelTypes.isObjectExpression(init)) {
                                    const got = init ? init.type : 'undefined';
                                    throw new Error(errorMessage(exportState, `Expected object but got ${got}`));
                                }
                                for (const prop of init.properties){
                                    if (BabelTypes.isSpreadElement(prop)) {
                                        throw new Error(errorMessage(exportState, `Property spread is not allowed`));
                                    }
                                    const { name } = prop.key;
                                    if (BabelTypes.isIdentifier(prop.key, {
                                        name: 'amp'
                                    })) {
                                        if (!BabelTypes.isObjectProperty(prop)) {
                                            throw new Error(errorMessage(exportState, `Invalid property "${name}"`));
                                        }
                                        if (!BabelTypes.isBooleanLiteral(prop.value) && !BabelTypes.isStringLiteral(prop.value)) {
                                            throw new Error(errorMessage(exportState, `Invalid value for "${name}"`));
                                        }
                                        config.amp = prop.value.value;
                                    }
                                }
                            }
                            if (config.amp === true) {
                                var _exportState_file_opts, _exportState_file;
                                if (!((_exportState_file = exportState.file) == null ? void 0 : (_exportState_file_opts = _exportState_file.opts) == null ? void 0 : _exportState_file_opts.caller.isDev)) {
                                    // don't replace bundle in development so HMR can track
                                    // dependencies and trigger reload when they are changed
                                    replaceBundle(exportPath, t);
                                }
                                exportState.bundleDropped = true;
                                return;
                            }
                        }
                    }, state);
                }
            }
        }
    };
}

//# sourceMappingURL=next-page-config.js.map