"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "SimpleWebpackError", {
    enumerable: true,
    get: function() {
        return SimpleWebpackError;
    }
});
class SimpleWebpackError extends Error {
    constructor(file, message){
        super(message);
        this.file = file;
    }
}

//# sourceMappingURL=simpleWebpackError.js.map