"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "DraftMode", {
    enumerable: true,
    get: function() {
        return DraftMode;
    }
});
const _staticgenerationasyncstorageexternal = require("./static-generation-async-storage.external");
const _dynamicrendering = require("../../server/app-render/dynamic-rendering");
class DraftMode {
    get isEnabled() {
        return this._provider.isEnabled;
    }
    enable() {
        const store = _staticgenerationasyncstorageexternal.staticGenerationAsyncStorage.getStore();
        if (store) {
            // We we have a store we want to track dynamic data access to ensure we
            // don't statically generate routes that manipulate draft mode.
            (0, _dynamicrendering.trackDynamicDataAccessed)(store, "draftMode().enable()");
        }
        return this._provider.enable();
    }
    disable() {
        const store = _staticgenerationasyncstorageexternal.staticGenerationAsyncStorage.getStore();
        if (store) {
            // We we have a store we want to track dynamic data access to ensure we
            // don't statically generate routes that manipulate draft mode.
            (0, _dynamicrendering.trackDynamicDataAccessed)(store, "draftMode().disable()");
        }
        return this._provider.disable();
    }
    constructor(provider){
        this._provider = provider;
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=draft-mode.js.map