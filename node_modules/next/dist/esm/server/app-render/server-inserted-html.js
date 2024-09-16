// Provider for the `useServerInsertedHTML` API to register callbacks to insert
// elements into the HTML stream.
import { jsx as _jsx } from "react/jsx-runtime";
import React from "react";
import { ServerInsertedHTMLContext } from "../../shared/lib/server-inserted-html.shared-runtime";
export function createServerInsertedHTML() {
    const serverInsertedHTMLCallbacks = [];
    const addInsertedHtml = (handler)=>{
        serverInsertedHTMLCallbacks.push(handler);
    };
    return {
        ServerInsertedHTMLProvider ({ children }) {
            return /*#__PURE__*/ _jsx(ServerInsertedHTMLContext.Provider, {
                value: addInsertedHtml,
                children: children
            });
        },
        renderServerInsertedHTML () {
            return serverInsertedHTMLCallbacks.map((callback, index)=>/*#__PURE__*/ _jsx(React.Fragment, {
                    children: callback()
                }, "__next_server_inserted__" + index));
        }
    };
}

//# sourceMappingURL=server-inserted-html.js.map