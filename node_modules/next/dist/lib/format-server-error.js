"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    formatServerError: null,
    getStackWithoutErrorMessage: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    formatServerError: function() {
        return formatServerError;
    },
    getStackWithoutErrorMessage: function() {
        return getStackWithoutErrorMessage;
    }
});
const invalidServerComponentReactHooks = [
    "useDeferredValue",
    "useEffect",
    "useImperativeHandle",
    "useInsertionEffect",
    "useLayoutEffect",
    "useReducer",
    "useRef",
    "useState",
    "useSyncExternalStore",
    "useTransition",
    "experimental_useOptimistic",
    "useOptimistic"
];
function setMessage(error, message) {
    error.message = message;
    if (error.stack) {
        const lines = error.stack.split("\n");
        lines[0] = message;
        error.stack = lines.join("\n");
    }
}
function getStackWithoutErrorMessage(error) {
    const stack = error.stack;
    if (!stack) return "";
    return stack.replace(/^[^\n]*\n/, "");
}
function formatServerError(error) {
    if (typeof (error == null ? void 0 : error.message) !== "string") return;
    if (error.message.includes("Class extends value undefined is not a constructor or null")) {
        const addedMessage = "This might be caused by a React Class Component being rendered in a Server Component, React Class Components only works in Client Components. Read more: https://nextjs.org/docs/messages/class-component-in-server-component";
        // If this error instance already has the message, don't add it again
        if (error.message.includes(addedMessage)) return;
        setMessage(error, `${error.message}

${addedMessage}`);
        return;
    }
    if (error.message.includes("createContext is not a function")) {
        setMessage(error, 'createContext only works in Client Components. Add the "use client" directive at the top of the file to use it. Read more: https://nextjs.org/docs/messages/context-in-server-component');
        return;
    }
    for (const clientHook of invalidServerComponentReactHooks){
        const regex = new RegExp(`\\b${clientHook}\\b.*is not a function`);
        if (regex.test(error.message)) {
            setMessage(error, `${clientHook} only works in Client Components. Add the "use client" directive at the top of the file to use it. Read more: https://nextjs.org/docs/messages/react-client-hook-in-server-component`);
            return;
        }
    }
}

//# sourceMappingURL=format-server-error.js.map