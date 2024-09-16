"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ABORT: null,
    CONTINUE: null,
    UNHANDLED: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ABORT: function() {
        return ABORT;
    },
    CONTINUE: function() {
        return CONTINUE;
    },
    UNHANDLED: function() {
        return UNHANDLED;
    }
});
const ABORT = {
    api: "abort"
};
const CONTINUE = {
    api: "continue"
};
const UNHANDLED = {
    api: "unhandled"
};

//# sourceMappingURL=types.js.map