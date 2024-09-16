"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    setGlobal: null,
    traceGlobals: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    setGlobal: function() {
        return setGlobal;
    },
    traceGlobals: function() {
        return traceGlobals;
    }
});
let _traceGlobals = global._traceGlobals;
if (!_traceGlobals) {
    _traceGlobals = new Map();
}
global._traceGlobals = _traceGlobals;
const traceGlobals = _traceGlobals;
const setGlobal = (key, val)=>{
    traceGlobals.set(key, val);
};

//# sourceMappingURL=shared.js.map