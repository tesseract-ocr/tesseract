"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "consumeIterator", {
    enumerable: true,
    get: function() {
        return consumeIterator;
    }
});
function consumeIterator(iter) {
    while(true){
        const { value, done } = iter.next();
        if (done) {
            return value;
        }
    }
}

//# sourceMappingURL=util.js.map