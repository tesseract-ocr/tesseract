// DO NOT MODIFY THIS FILE DIRECTLY
// It's for aliasing the `semver` package to be a noop for the `jsonwebtoken` package.
// We're trying to minimize the size of the worker bundle.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "satisfies", {
    enumerable: true,
    get: function() {
        return satisfies;
    }
});
function satisfies() {
    return true;
}

//# sourceMappingURL=semver-noop.js.map