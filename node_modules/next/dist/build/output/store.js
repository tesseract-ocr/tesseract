"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    formatTrigger: null,
    store: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    formatTrigger: function() {
        return formatTrigger;
    },
    store: function() {
        return store;
    }
});
const _unistore = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/unistore"));
const _stripansi = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/strip-ansi"));
const _trace = require("../../trace");
const _swc = require("../swc");
const _log = /*#__PURE__*/ _interop_require_wildcard(require("./log"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function _getRequireWildcardCache(nodeInterop) {
    if (typeof WeakMap !== "function") return null;
    var cacheBabelInterop = new WeakMap();
    var cacheNodeInterop = new WeakMap();
    return (_getRequireWildcardCache = function(nodeInterop) {
        return nodeInterop ? cacheNodeInterop : cacheBabelInterop;
    })(nodeInterop);
}
function _interop_require_wildcard(obj, nodeInterop) {
    if (!nodeInterop && obj && obj.__esModule) {
        return obj;
    }
    if (obj === null || typeof obj !== "object" && typeof obj !== "function") {
        return {
            default: obj
        };
    }
    var cache = _getRequireWildcardCache(nodeInterop);
    if (cache && cache.has(obj)) {
        return cache.get(obj);
    }
    var newObj = {
        __proto__: null
    };
    var hasPropertyDescriptor = Object.defineProperty && Object.getOwnPropertyDescriptor;
    for(var key in obj){
        if (key !== "default" && Object.prototype.hasOwnProperty.call(obj, key)) {
            var desc = hasPropertyDescriptor ? Object.getOwnPropertyDescriptor(obj, key) : null;
            if (desc && (desc.get || desc.set)) {
                Object.defineProperty(newObj, key, desc);
            } else {
                newObj[key] = obj[key];
            }
        }
    }
    newObj.default = obj;
    if (cache) {
        cache.set(obj, newObj);
    }
    return newObj;
}
const MAX_LOG_SKIP_DURATION = 500 // 500ms
;
function formatTrigger(trigger) {
    // Format dynamic sitemap routes to simpler file path
    // e.g., /sitemap.xml[] -> /sitemap.xml
    if (trigger.includes('[__metadata_id__]')) {
        trigger = trigger.replace('/[__metadata_id__]', '/[id]');
    }
    if (trigger.length > 1 && trigger.endsWith('/')) {
        trigger = trigger.slice(0, -1);
    }
    return trigger;
}
const store = (0, _unistore.default)({
    appUrl: null,
    bindAddr: null,
    bootstrap: true,
    logging: true
});
let lastStore = {
    appUrl: null,
    bindAddr: null,
    bootstrap: true,
    logging: true
};
function hasStoreChanged(nextStore) {
    if ([
        ...new Set([
            ...Object.keys(lastStore),
            ...Object.keys(nextStore)
        ])
    ].every((key)=>Object.is(lastStore[key], nextStore[key]))) {
        return false;
    }
    lastStore = nextStore;
    return true;
}
let startTime = 0;
let trigger = '' // default, use empty string for trigger
;
let triggerUrl = undefined;
let loadingLogTimer = null;
let traceSpan = null;
let logging = true;
store.subscribe((state)=>{
    // Update persisted logging state
    if ('logging' in state) {
        logging = state.logging;
    }
    // If logging is disabled, do not log
    if (!logging) {
        return;
    }
    if (!hasStoreChanged(state)) {
        return;
    }
    if (state.bootstrap) {
        return;
    }
    if (state.loading) {
        if (state.trigger) {
            trigger = formatTrigger(state.trigger);
            triggerUrl = state.url;
            if (trigger !== 'initial') {
                traceSpan = (0, _trace.trace)('compile-path', undefined, {
                    trigger: trigger
                });
                if (!loadingLogTimer) {
                    // Only log compiling if compiled is not finished in 3 seconds
                    loadingLogTimer = setTimeout(()=>{
                        if (triggerUrl && triggerUrl !== trigger && process.env.NEXT_TRIGGER_URL) {
                            _log.wait(`Compiling ${trigger} (${triggerUrl}) ...`);
                        } else {
                            _log.wait(`Compiling ${trigger} ...`);
                        }
                    }, MAX_LOG_SKIP_DURATION);
                }
            }
        }
        if (startTime === 0) {
            startTime = Date.now();
        }
        return;
    }
    if (state.errors) {
        // Log compilation errors
        _log.error(state.errors[0]);
        const cleanError = (0, _stripansi.default)(state.errors[0]);
        if (cleanError.indexOf('SyntaxError') > -1) {
            const matches = cleanError.match(/\[.*\]=/);
            if (matches) {
                for (const match of matches){
                    const prop = (match.split(']').shift() || '').slice(1);
                    console.log(`AMP bind syntax [${prop}]='' is not supported in JSX, use 'data-amp-bind-${prop}' instead. https://nextjs.org/docs/messages/amp-bind-jsx-alt`);
                }
                return;
            }
        }
        startTime = 0;
        // Ensure traces are flushed after each compile in development mode
        (0, _trace.flushAllTraces)();
        (0, _swc.teardownTraceSubscriber)();
        (0, _swc.teardownHeapProfiler)();
        return;
    }
    let timeMessage = '';
    if (startTime) {
        const time = Date.now() - startTime;
        startTime = 0;
        timeMessage = ' ' + (time > 2000 ? `in ${Math.round(time / 100) / 10}s` : `in ${time}ms`);
    }
    let modulesMessage = '';
    if (state.totalModulesCount) {
        modulesMessage = ` (${state.totalModulesCount} modules)`;
    }
    if (state.warnings) {
        _log.warn(state.warnings.join('\n\n'));
        // Ensure traces are flushed after each compile in development mode
        (0, _trace.flushAllTraces)();
        (0, _swc.teardownTraceSubscriber)();
        (0, _swc.teardownHeapProfiler)();
        return;
    }
    if (state.typeChecking) {
        _log.info(`bundled ${trigger}${timeMessage}${modulesMessage}, type checking...`);
        return;
    }
    if (trigger === 'initial') {
        trigger = '';
    } else {
        if (loadingLogTimer) {
            clearTimeout(loadingLogTimer);
            loadingLogTimer = null;
        }
        if (traceSpan) {
            traceSpan.stop();
            traceSpan = null;
        }
        _log.event(`Compiled${trigger ? ' ' + trigger : ''}${timeMessage}${modulesMessage}`);
        trigger = '';
    }
    // Ensure traces are flushed after each compile in development mode
    (0, _trace.flushAllTraces)();
    (0, _swc.teardownTraceSubscriber)();
    (0, _swc.teardownHeapProfiler)();
});

//# sourceMappingURL=store.js.map