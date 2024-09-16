/**
 * `app` -> app dir
 * `pages` -> pages dir
 * `root` -> middleware / instrumentation
 * `assets` -> assets
 */ "use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    getEntryKey: null,
    splitEntryKey: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    getEntryKey: function() {
        return getEntryKey;
    },
    splitEntryKey: function() {
        return splitEntryKey;
    }
});
function getEntryKey(type, side, page) {
    return JSON.stringify({
        type,
        side,
        page
    });
}
function splitEntryKey(key) {
    return JSON.parse(key);
}

//# sourceMappingURL=entry-key.js.map