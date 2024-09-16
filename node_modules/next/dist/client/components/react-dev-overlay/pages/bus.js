"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    emit: null,
    off: null,
    on: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    emit: function() {
        return emit;
    },
    off: function() {
        return off;
    },
    on: function() {
        return on;
    }
});
let handlers = new Set();
let queue = [];
function drain() {
    // Draining should never happen synchronously in case multiple handlers are
    // registered.
    setTimeout(function() {
        while(// Until we are out of events:
        Boolean(queue.length) && // Or, if all handlers removed themselves as a result of handling the
        // event(s)
        Boolean(handlers.size)){
            const ev = queue.shift();
            handlers.forEach((handler)=>handler(ev));
        }
    }, 1);
}
function emit(ev) {
    queue.push(Object.freeze({
        ...ev
    }));
    drain();
}
function on(fn) {
    if (handlers.has(fn)) {
        return false;
    }
    handlers.add(fn);
    drain();
    return true;
}
function off(fn) {
    if (handlers.has(fn)) {
        handlers.delete(fn);
        return true;
    }
    return false;
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=bus.js.map