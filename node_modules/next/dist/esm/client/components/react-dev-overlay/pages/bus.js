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
export function emit(ev) {
    queue.push(Object.freeze({
        ...ev
    }));
    drain();
}
export function on(fn) {
    if (handlers.has(fn)) {
        return false;
    }
    handlers.add(fn);
    drain();
    return true;
}
export function off(fn) {
    if (handlers.has(fn)) {
        handlers.delete(fn);
        return true;
    }
    return false;
}

//# sourceMappingURL=bus.js.map