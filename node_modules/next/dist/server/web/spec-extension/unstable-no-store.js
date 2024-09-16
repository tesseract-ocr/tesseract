"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "unstable_noStore", {
    enumerable: true,
    get: function() {
        return unstable_noStore;
    }
});
const _staticgenerationasyncstorageexternal = require("../../../client/components/static-generation-async-storage.external");
const _dynamicrendering = require("../../app-render/dynamic-rendering");
function unstable_noStore() {
    const callingExpression = "unstable_noStore()";
    const store = _staticgenerationasyncstorageexternal.staticGenerationAsyncStorage.getStore();
    if (!store) {
        // This generally implies we are being called in Pages router. We should probably not support
        // unstable_noStore in contexts outside of `react-server` condition but since we historically
        // have not errored here previously, we maintain that behavior for now.
        return;
    } else if (store.forceStatic) {
        return;
    } else {
        store.isUnstableNoStore = true;
        (0, _dynamicrendering.markCurrentScopeAsDynamic)(store, callingExpression);
    }
}

//# sourceMappingURL=unstable-no-store.js.map