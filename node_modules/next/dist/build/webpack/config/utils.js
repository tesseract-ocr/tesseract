"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "pipe", {
    enumerable: true,
    get: function() {
        return pipe;
    }
});
const pipe = (...fns)=>(param)=>fns.reduce(async (result, next)=>next(await result), param);

//# sourceMappingURL=utils.js.map