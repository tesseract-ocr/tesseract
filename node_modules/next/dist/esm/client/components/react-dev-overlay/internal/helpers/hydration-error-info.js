import { getHydrationErrorStackInfo } from '../../../is-hydration-error';
export const hydrationErrorState = {};
// https://github.com/facebook/react/blob/main/packages/react-dom/src/__tests__/ReactDOMHydrationDiff-test.js used as a reference
const htmlTagsWarnings = new Set([
    'Warning: In HTML, %s cannot be a child of <%s>.%s\nThis will cause a hydration error.%s',
    'Warning: In HTML, %s cannot be a descendant of <%s>.\nThis will cause a hydration error.%s',
    'Warning: In HTML, text nodes cannot be a child of <%s>.\nThis will cause a hydration error.',
    "Warning: In HTML, whitespace text nodes cannot be a child of <%s>. Make sure you don't have any extra whitespace between tags on each line of your source code.\nThis will cause a hydration error.",
    'Warning: Expected server HTML to contain a matching <%s> in <%s>.%s',
    'Warning: Did not expect server HTML to contain a <%s> in <%s>.%s'
]);
const textAndTagsMismatchWarnings = new Set([
    'Warning: Expected server HTML to contain a matching text node for "%s" in <%s>.%s',
    'Warning: Did not expect server HTML to contain the text node "%s" in <%s>.%s'
]);
const textMismatchWarning = 'Warning: Text content did not match. Server: "%s" Client: "%s"%s';
export const getHydrationWarningType = (message)=>{
    if (typeof message !== 'string') {
        // TODO: Doesn't make sense to treat no message as a hydration error message.
        // We should bail out somewhere earlier.
        return 'text';
    }
    const normalizedMessage = message.startsWith('Warning: ') ? message : "Warning: " + message;
    if (isHtmlTagsWarning(normalizedMessage)) return 'tag';
    if (isTextInTagsMismatchWarning(normalizedMessage)) return 'text-in-tag';
    return 'text';
};
const isHtmlTagsWarning = (message)=>htmlTagsWarnings.has(message);
const isTextMismatchWarning = (message)=>textMismatchWarning === message;
const isTextInTagsMismatchWarning = (msg)=>textAndTagsMismatchWarnings.has(msg);
const isKnownHydrationWarning = (message)=>{
    if (typeof message !== 'string') {
        return false;
    }
    // React 18 has the `Warning: ` prefix.
    // React 19 does not.
    const normalizedMessage = message.startsWith('Warning: ') ? message : "Warning: " + message;
    return isHtmlTagsWarning(normalizedMessage) || isTextInTagsMismatchWarning(normalizedMessage) || isTextMismatchWarning(normalizedMessage);
};
export const getReactHydrationDiffSegments = (msg)=>{
    if (msg) {
        const { message, diff } = getHydrationErrorStackInfo(msg);
        if (message) return [
            message,
            diff
        ];
    }
    return undefined;
};
/**
 * Patch console.error to capture hydration errors.
 * If any of the knownHydrationWarnings are logged, store the message and component stack.
 * When the hydration runtime error is thrown, the message and component stack are added to the error.
 * This results in a more helpful error message in the error overlay.
 */ export function storeHydrationErrorStateFromConsoleArgs() {
    for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
        args[_key] = arguments[_key];
    }
    const [msg, serverContent, clientContent, componentStack] = args;
    if (isKnownHydrationWarning(msg)) {
        hydrationErrorState.warning = [
            // remove the last %s from the message
            msg,
            serverContent,
            clientContent
        ];
        hydrationErrorState.componentStack = componentStack;
        hydrationErrorState.serverContent = serverContent;
        hydrationErrorState.clientContent = clientContent;
    }
}

//# sourceMappingURL=hydration-error-info.js.map