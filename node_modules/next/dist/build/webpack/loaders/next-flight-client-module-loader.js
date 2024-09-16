"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return transformSource;
    }
});
const _getpagestaticinfo = require("../../analysis/get-page-static-info");
const _getmodulebuildinfo = require("./get-module-build-info");
function transformSource(source, sourceMap) {
    // Avoid buffer to be consumed
    if (typeof source !== "string") {
        throw new Error("Expected source to have been transformed to a string.");
    }
    // Assign the RSC meta information to buildInfo.
    const buildInfo = (0, _getmodulebuildinfo.getModuleBuildInfo)(this._module);
    buildInfo.rsc = (0, _getpagestaticinfo.getRSCModuleInformation)(source, false);
    // This is a server action entry module in the client layer. We need to attach
    // noop exports of `callServer` wrappers for each action.
    if (buildInfo.rsc.actions) {
        source = `
import { callServer } from 'next/dist/client/app-call-server'

function __build_action__(action, args) {
  return callServer(action.$$id, args)
}

${source}
`;
    }
    return this.callback(null, source, sourceMap);
}

//# sourceMappingURL=next-flight-client-module-loader.js.map