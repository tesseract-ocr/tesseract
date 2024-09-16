import { STRING_LITERAL_DROP_BUNDLE } from "../../../shared/lib/constants";
export const ampFirstEntryNamesMap = new WeakMap();
const PLUGIN_NAME = "DropAmpFirstPagesPlugin";
// Prevents outputting client pages when they are not needed
export class DropClientPage {
    apply(compiler) {
        compiler.hooks.compilation.tap(PLUGIN_NAME, (compilation, { normalModuleFactory })=>{
            // Recursively look up the issuer till it ends up at the root
            function findEntryModule(mod) {
                const queue = new Set([
                    mod
                ]);
                for (const module of queue){
                    const incomingConnections = compilation.moduleGraph.getIncomingConnections(module);
                    for (const incomingConnection of incomingConnections){
                        if (!incomingConnection.originModule) return module;
                        queue.add(incomingConnection.originModule);
                    }
                }
                return null;
            }
            function handler(parser) {
                function markAsAmpFirst() {
                    const entryModule = findEntryModule(parser.state.module);
                    if (!entryModule) {
                        return;
                    }
                    // @ts-ignore buildInfo exists on Module
                    entryModule.buildInfo.NEXT_ampFirst = true;
                }
                parser.hooks.preDeclarator.tap(PLUGIN_NAME, (declarator)=>{
                    var _declarator_id;
                    if ((declarator == null ? void 0 : (_declarator_id = declarator.id) == null ? void 0 : _declarator_id.name) === STRING_LITERAL_DROP_BUNDLE) {
                        markAsAmpFirst();
                    }
                });
            }
            normalModuleFactory.hooks.parser.for("javascript/auto").tap(PLUGIN_NAME, handler);
            normalModuleFactory.hooks.parser.for("javascript/esm").tap(PLUGIN_NAME, handler);
            normalModuleFactory.hooks.parser.for("javascript/dynamic").tap(PLUGIN_NAME, handler);
            if (!ampFirstEntryNamesMap.has(compilation)) {
                ampFirstEntryNamesMap.set(compilation, []);
            }
            const ampFirstEntryNamesItem = ampFirstEntryNamesMap.get(compilation);
            compilation.hooks.seal.tap(PLUGIN_NAME, ()=>{
                for (const [name, entryData] of compilation.entries){
                    for (const dependency of entryData.dependencies){
                        var _module_buildInfo;
                        const module = compilation.moduleGraph.getModule(dependency);
                        if (module == null ? void 0 : (_module_buildInfo = module.buildInfo) == null ? void 0 : _module_buildInfo.NEXT_ampFirst) {
                            ampFirstEntryNamesItem.push(name);
                            compilation.entries.delete(name);
                        }
                    }
                }
            });
        });
    }
    constructor(){
        this.ampPages = new Set();
    }
}

//# sourceMappingURL=next-drop-client-page-plugin.js.map