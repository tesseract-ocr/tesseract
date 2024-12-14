"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ALLOWED_EXPORTS: null,
    ALLOWED_LAYOUT_PROPS: null,
    ALLOWED_PAGE_PROPS: null,
    DISALLOWED_SERVER_REACT_APIS: null,
    DISALLOWED_SERVER_REACT_DOM_APIS: null,
    LEGACY_CONFIG_EXPORT: null,
    NEXT_TS_ERRORS: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ALLOWED_EXPORTS: function() {
        return ALLOWED_EXPORTS;
    },
    ALLOWED_LAYOUT_PROPS: function() {
        return ALLOWED_LAYOUT_PROPS;
    },
    ALLOWED_PAGE_PROPS: function() {
        return ALLOWED_PAGE_PROPS;
    },
    DISALLOWED_SERVER_REACT_APIS: function() {
        return DISALLOWED_SERVER_REACT_APIS;
    },
    DISALLOWED_SERVER_REACT_DOM_APIS: function() {
        return DISALLOWED_SERVER_REACT_DOM_APIS;
    },
    LEGACY_CONFIG_EXPORT: function() {
        return LEGACY_CONFIG_EXPORT;
    },
    NEXT_TS_ERRORS: function() {
        return NEXT_TS_ERRORS;
    }
});
const NEXT_TS_ERRORS = {
    INVALID_SERVER_API: 71001,
    INVALID_ENTRY_EXPORT: 71002,
    INVALID_OPTION_VALUE: 71003,
    MISPLACED_ENTRY_DIRECTIVE: 71004,
    INVALID_PAGE_PROP: 71005,
    INVALID_CONFIG_OPTION: 71006,
    INVALID_CLIENT_ENTRY_PROP: 71007,
    INVALID_METADATA_EXPORT: 71008,
    INVALID_ERROR_COMPONENT: 71009,
    INVALID_ENTRY_DIRECTIVE: 71010,
    INVALID_SERVER_ENTRY_RETURN: 71011
};
const ALLOWED_EXPORTS = [
    'config',
    'generateStaticParams',
    'metadata',
    'generateMetadata',
    'viewport',
    'generateViewport'
];
const LEGACY_CONFIG_EXPORT = 'config';
const DISALLOWED_SERVER_REACT_APIS = [
    'useState',
    'useEffect',
    'useLayoutEffect',
    'useDeferredValue',
    'useImperativeHandle',
    'useInsertionEffect',
    'useReducer',
    'useRef',
    'useSyncExternalStore',
    'useTransition',
    'Component',
    'PureComponent',
    'createContext',
    'createFactory',
    'experimental_useOptimistic',
    'useOptimistic',
    'useActionState'
];
const DISALLOWED_SERVER_REACT_DOM_APIS = [
    'useFormStatus',
    'useFormState'
];
const ALLOWED_PAGE_PROPS = [
    'params',
    'searchParams'
];
const ALLOWED_LAYOUT_PROPS = [
    'params',
    'children'
];

//# sourceMappingURL=constant.js.map