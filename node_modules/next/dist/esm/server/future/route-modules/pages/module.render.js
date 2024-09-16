export const lazyRenderPagesPage = (...args)=>{
    if (process.env.NEXT_MINIMAL) {
        throw new Error("Can't use lazyRenderPagesPage in minimal mode");
    } else {
        const render = require("./module.compiled").renderToHTML;
        return render(...args);
    }
};

//# sourceMappingURL=module.render.js.map