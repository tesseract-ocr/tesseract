"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    HtmlContext: null,
    useHtmlContext: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    HtmlContext: function() {
        return HtmlContext;
    },
    useHtmlContext: function() {
        return useHtmlContext;
    }
});
const _react = require("react");
const HtmlContext = (0, _react.createContext)(undefined);
if (process.env.NODE_ENV !== 'production') {
    HtmlContext.displayName = 'HtmlContext';
}
function useHtmlContext() {
    const context = (0, _react.useContext)(HtmlContext);
    if (!context) {
        throw new Error("<Html> should not be imported outside of pages/_document.\n" + 'Read more: https://nextjs.org/docs/messages/no-document-import-in-page');
    }
    return context;
}

//# sourceMappingURL=html-context.shared-runtime.js.map