import { staticGenerationAsyncStorage } from "./static-generation-async-storage.external";
import { trackDynamicDataAccessed } from "../../server/app-render/dynamic-rendering";
export class DraftMode {
    get isEnabled() {
        return this._provider.isEnabled;
    }
    enable() {
        const store = staticGenerationAsyncStorage.getStore();
        if (store) {
            // We we have a store we want to track dynamic data access to ensure we
            // don't statically generate routes that manipulate draft mode.
            trackDynamicDataAccessed(store, "draftMode().enable()");
        }
        return this._provider.enable();
    }
    disable() {
        const store = staticGenerationAsyncStorage.getStore();
        if (store) {
            // We we have a store we want to track dynamic data access to ensure we
            // don't statically generate routes that manipulate draft mode.
            trackDynamicDataAccessed(store, "draftMode().disable()");
        }
        return this._provider.disable();
    }
    constructor(provider){
        this._provider = provider;
    }
}

//# sourceMappingURL=draft-mode.js.map