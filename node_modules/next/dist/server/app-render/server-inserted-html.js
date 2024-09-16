// Provider for the `useServerInsertedHTML` API to register callbacks to insert
// elements into the HTML stream.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createServerInsertedHTML", {
    enumerable: true,
    get: function() {
        return createServerInsertedHTML;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _react = /*#__PURE__*/ _interop_require_default(require("react"));
const _serverinsertedhtmlsharedruntime = require("../../shared/lib/server-inserted-html.shared-runtime");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function createServerInsertedHTML() {
    const serverInsertedHTMLCallbacks = [];
    const addInsertedHtml = (handler)=>{
        serverInsertedHTMLCallbacks.push(handler);
    };
    return {
        ServerInsertedHTMLProvider ({ children }) {
            return /*#__PURE__*/ (0, _jsxruntime.jsx)(_serverinsertedhtmlsharedruntime.ServerInsertedHTMLContext.Provider, {
                value: addInsertedHtml,
                children: children
            });
        },
        renderServerInsertedHTML () {
            return serverInsertedHTMLCallbacks.map((callback, index)=>/*#__PURE__*/ (0, _jsxruntime.jsx)(_react.default.Fragment, {
                    children: callback()
                }, "__next_server_inserted__" + index));
        }
    };
}

//# sourceMappingURL=server-inserted-html.js.map