"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    createServerModuleMap: null,
    selectWorkerForForwarding: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    createServerModuleMap: function() {
        return createServerModuleMap;
    },
    selectWorkerForForwarding: function() {
        return selectWorkerForForwarding;
    }
});
const _apppaths = require("../../shared/lib/router/utils/app-paths");
const _pathhasprefix = require("../../shared/lib/router/utils/path-has-prefix");
const _removepathprefix = require("../../shared/lib/router/utils/remove-path-prefix");
function createServerModuleMap({ serverActionsManifest, pageName }) {
    return new Proxy({}, {
        get: (_, id)=>{
            return {
                id: serverActionsManifest[process.env.NEXT_RUNTIME === "edge" ? "edge" : "node"][id].workers[normalizeWorkerPageName(pageName)],
                name: id,
                chunks: []
            };
        }
    });
}
function selectWorkerForForwarding(actionId, pageName, serverActionsManifest) {
    var _serverActionsManifest__actionId;
    const workers = (_serverActionsManifest__actionId = serverActionsManifest[process.env.NEXT_RUNTIME === "edge" ? "edge" : "node"][actionId]) == null ? void 0 : _serverActionsManifest__actionId.workers;
    const workerName = normalizeWorkerPageName(pageName);
    // no workers, nothing to forward to
    if (!workers) return;
    // if there is a worker for this page, no need to forward it.
    if (workers[workerName]) {
        return;
    }
    // otherwise, grab the first worker that has a handler for this action id
    return denormalizeWorkerPageName(Object.keys(workers)[0]);
}
/**
 * The flight entry loader keys actions by bundlePath.
 * bundlePath corresponds with the relative path (including 'app') to the page entrypoint.
 */ function normalizeWorkerPageName(pageName) {
    if ((0, _pathhasprefix.pathHasPrefix)(pageName, "app")) {
        return pageName;
    }
    return "app" + pageName;
}
/**
 * Converts a bundlePath (relative path to the entrypoint) to a routable page name
 */ function denormalizeWorkerPageName(bundlePath) {
    return (0, _apppaths.normalizeAppPath)((0, _removepathprefix.removePathPrefix)(bundlePath, "app"));
}

//# sourceMappingURL=action-utils.js.map