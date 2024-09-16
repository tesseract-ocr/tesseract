"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "lazyRenderPagesPage", {
    enumerable: true,
    get: function() {
        return lazyRenderPagesPage;
    }
});
const lazyRenderPagesPage = (...args)=>{
    if (process.env.NEXT_MINIMAL) {
        throw new Error("Can't use lazyRenderPagesPage in minimal mode");
    } else {
        const render = require("./module.compiled").renderToHTML;
        return render(...args);
    }
};

//# sourceMappingURL=module.render.js.map