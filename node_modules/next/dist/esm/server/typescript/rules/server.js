import { DISALLOWED_SERVER_REACT_APIS, DISALLOWED_SERVER_REACT_DOM_APIS, NEXT_TS_ERRORS } from '../constant';
import { getTs } from '../utils';
const serverLayer = {
    // On the server layer we need to filter out some invalid completion results.
    filterCompletionsAtPosition (entries) {
        return entries.filter((e)=>{
            // Remove disallowed React APIs.
            if (DISALLOWED_SERVER_REACT_APIS.includes(e.name) && e.source === 'react') {
                return false;
            }
            return true;
        });
    },
    // Filter out quick info for some React APIs.
    hasDisallowedReactAPIDefinition (definitions) {
        return definitions == null ? void 0 : definitions.some((d)=>DISALLOWED_SERVER_REACT_APIS.includes(d.name) && d.containerName === 'React');
    },
    // Give errors about disallowed imports such as `useState`.
    getSemanticDiagnosticsForImportDeclaration (source, node) {
        const ts = getTs();
        const diagnostics = [];
        const importPath = node.moduleSpecifier.getText(source);
        const importClause = node.importClause;
        const namedBindings = importClause == null ? void 0 : importClause.namedBindings;
        if (importClause) {
            if (/^['"]react['"]$/.test(importPath)) {
                // Check if it imports "useState"
                if (namedBindings && ts.isNamedImports(namedBindings)) {
                    const elements = namedBindings.elements;
                    for (const element of elements){
                        const name = element.name.getText(source);
                        if (DISALLOWED_SERVER_REACT_APIS.includes(name)) {
                            diagnostics.push({
                                file: source,
                                category: ts.DiagnosticCategory.Error,
                                code: NEXT_TS_ERRORS.INVALID_SERVER_API,
                                messageText: `"${name}" is not allowed in Server Components.`,
                                start: element.name.getStart(),
                                length: element.name.getWidth()
                            });
                        }
                    }
                }
            } else if (/^['"]react-dom['"]$/.test(importPath)) {
                // Check if it imports "useFormState"
                if (namedBindings && ts.isNamedImports(namedBindings)) {
                    const elements = namedBindings.elements;
                    for (const element of elements){
                        const name = element.name.getText(source);
                        if (DISALLOWED_SERVER_REACT_DOM_APIS.includes(name)) {
                            diagnostics.push({
                                file: source,
                                category: ts.DiagnosticCategory.Error,
                                code: NEXT_TS_ERRORS.INVALID_SERVER_API,
                                messageText: `"${name}" is not allowed in Server Components.`,
                                start: element.name.getStart(),
                                length: element.name.getWidth()
                            });
                        }
                    }
                }
            }
        }
        return diagnostics;
    }
};
export default serverLayer;

//# sourceMappingURL=server.js.map