"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "default", {
    enumerable: true,
    get: function() {
        return _default;
    }
});
const _stringifyrequest = require("../stringify-request");
// this parameter: https://www.typescriptlang.org/docs/handbook/functions.html#this-parameters
function nextClientPagesLoader() {
    const pagesLoaderSpan = this.currentTraceSpan.traceChild("next-client-pages-loader");
    return pagesLoaderSpan.traceFn(()=>{
        const { absolutePagePath, page } = this.getOptions();
        pagesLoaderSpan.setAttribute("absolutePagePath", absolutePagePath);
        const stringifiedPageRequest = (0, _stringifyrequest.stringifyRequest)(this, absolutePagePath);
        const stringifiedPage = JSON.stringify(page);
        return `
    (window.__NEXT_P = window.__NEXT_P || []).push([
      ${stringifiedPage},
      function () {
        return require(${stringifiedPageRequest});
      }
    ]);
    if(module.hot) {
      module.hot.dispose(function () {
        window.__NEXT_P.push([${stringifiedPage}])
      });
    }
  `;
    });
}
const _default = nextClientPagesLoader;

//# sourceMappingURL=next-client-pages-loader.js.map