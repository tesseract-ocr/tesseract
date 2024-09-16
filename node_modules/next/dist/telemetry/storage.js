"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "Telemetry", {
    enumerable: true,
    get: function() {
        return Telemetry;
    }
});
const _picocolors = require("../lib/picocolors");
const _conf = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/conf"));
const _crypto = require("crypto");
const _isdocker = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/is-docker"));
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _anonymousmeta = require("./anonymous-meta");
const _ciinfo = /*#__PURE__*/ _interop_require_wildcard(require("./ci-info"));
const _postpayload = require("./post-payload");
const _projectid = require("./project-id");
const _ponyfill = require("next/dist/compiled/@edge-runtime/ponyfill");
const _fs = /*#__PURE__*/ _interop_require_default(require("fs"));
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
// This is the key that stores whether or not telemetry is enabled or disabled.
const TELEMETRY_KEY_ENABLED = "telemetry.enabled";
// This is the key that specifies when the user was informed about anonymous
// telemetry collection.
const TELEMETRY_KEY_NOTIFY_DATE = "telemetry.notifiedAt";
// This is a quasi-persistent identifier used to dedupe recurring events. It's
// generated from random data and completely anonymous.
const TELEMETRY_KEY_ID = `telemetry.anonymousId`;
// This is the cryptographic salt that is included within every hashed value.
// This salt value is never sent to us, ensuring privacy and the one-way nature
// of the hash (prevents dictionary lookups of pre-computed hashes).
// See the `oneWayHash` function.
const TELEMETRY_KEY_SALT = `telemetry.salt`;
function getStorageDirectory(distDir) {
    const isLikelyEphemeral = _ciinfo.isCI || (0, _isdocker.default)();
    if (isLikelyEphemeral) {
        return _path.default.join(distDir, "cache");
    }
    return undefined;
}
class Telemetry {
    constructor({ distDir }){
        this.notify = ()=>{
            if (this.isDisabled || !this.conf) {
                return;
            }
            // The end-user has already been notified about our telemetry integration. We
            // don't need to constantly annoy them about it.
            // We will re-inform users about the telemetry if significant changes are
            // ever made.
            if (this.conf.get(TELEMETRY_KEY_NOTIFY_DATE, "")) {
                return;
            }
            this.conf.set(TELEMETRY_KEY_NOTIFY_DATE, Date.now().toString());
            console.log(`${(0, _picocolors.magenta)((0, _picocolors.bold)("Attention"))}: Next.js now collects completely anonymous telemetry regarding usage.`);
            console.log(`This information is used to shape Next.js' roadmap and prioritize features.`);
            console.log(`You can learn more, including how to opt-out if you'd not like to participate in this anonymous program, by visiting the following URL:`);
            console.log((0, _picocolors.cyan)("https://nextjs.org/telemetry"));
            console.log();
        };
        this.setEnabled = (_enabled)=>{
            const enabled = !!_enabled;
            this.conf && this.conf.set(TELEMETRY_KEY_ENABLED, enabled);
            return this.conf && this.conf.path;
        };
        this.oneWayHash = (payload)=>{
            const hash = (0, _crypto.createHash)("sha256");
            // Always prepend the payload value with salt. This ensures the hash is truly
            // one-way.
            hash.update(this.salt);
            // Update is an append operation, not a replacement. The salt from the prior
            // update is still present!
            hash.update(payload);
            return hash.digest("hex");
        };
        this.record = (_events, deferred)=>{
            const prom = (deferred ? // flushDetached we can skip starting the initial
            // submitRecord which will then be cancelled
            new Promise((resolve)=>resolve({
                    isFulfilled: true,
                    isRejected: false,
                    value: _events
                })) : this.submitRecord(_events)).then((value)=>({
                    isFulfilled: true,
                    isRejected: false,
                    value
                })).catch((reason)=>({
                    isFulfilled: false,
                    isRejected: true,
                    reason
                }))// Acts as `Promise#finally` because `catch` transforms the error
            .then((res)=>{
                // Clean up the event to prevent unbounded `Set` growth
                if (!deferred) {
                    this.queue.delete(prom);
                }
                return res;
            });
            prom._events = Array.isArray(_events) ? _events : [
                _events
            ];
            prom._controller = prom._controller;
            // Track this `Promise` so we can flush pending events
            this.queue.add(prom);
            return prom;
        };
        this.flush = async ()=>Promise.all(this.queue).catch(()=>null);
        // writes current events to disk and spawns separate
        // detached process to submit the records without blocking
        // the main process from exiting
        this.flushDetached = (mode, dir)=>{
            const allEvents = [];
            this.queue.forEach((item)=>{
                try {
                    var _item__controller;
                    (_item__controller = item._controller) == null ? void 0 : _item__controller.abort();
                    allEvents.push(...item._events);
                } catch (_) {
                // if we fail to abort ignore this event
                }
            });
            _fs.default.mkdirSync(this.distDir, {
                recursive: true
            });
            _fs.default.writeFileSync(_path.default.join(this.distDir, "_events.json"), JSON.stringify(allEvents));
            // Note: cross-spawn is not used here as it causes
            // a new command window to appear when we don't want it to
            const child_process = require("child_process");
            // we use spawnSync when debugging to ensure logs are piped
            // correctly to stdout/stderr
            const spawn = this.NEXT_TELEMETRY_DEBUG ? child_process.spawnSync : child_process.spawn;
            spawn(process.execPath, [
                require.resolve("./detached-flush"),
                mode,
                dir
            ], {
                detached: !this.NEXT_TELEMETRY_DEBUG,
                windowsHide: true,
                shell: false,
                ...this.NEXT_TELEMETRY_DEBUG ? {
                    stdio: "inherit"
                } : {}
            });
        };
        this.submitRecord = async (_events)=>{
            let events;
            if (Array.isArray(_events)) {
                events = _events;
            } else {
                events = [
                    _events
                ];
            }
            if (events.length < 1) {
                return Promise.resolve();
            }
            if (this.NEXT_TELEMETRY_DEBUG) {
                // Print to standard error to simplify selecting the output
                events.forEach(({ eventName, payload })=>console.error(`[telemetry] ` + JSON.stringify({
                        eventName,
                        payload
                    }, null, 2)));
                // Do not send the telemetry data if debugging. Users may use this feature
                // to preview what data would be sent.
                return Promise.resolve();
            }
            // Skip recording telemetry if the feature is disabled
            if (this.isDisabled) {
                return Promise.resolve();
            }
            const context = {
                anonymousId: this.anonymousId,
                projectId: await this.getProjectId(),
                sessionId: this.sessionId
            };
            const meta = (0, _anonymousmeta.getAnonymousMeta)();
            const postController = new _ponyfill.AbortController();
            const res = (0, _postpayload._postPayload)(`https://telemetry.nextjs.org/api/v1/record`, {
                context,
                meta,
                events: events.map(({ eventName, payload })=>({
                        eventName,
                        fields: payload
                    }))
            }, postController.signal);
            res._controller = postController;
            return res;
        };
        // Read in the constructor so that .env can be loaded before reading
        const { NEXT_TELEMETRY_DISABLED, NEXT_TELEMETRY_DEBUG } = process.env;
        this.NEXT_TELEMETRY_DISABLED = NEXT_TELEMETRY_DISABLED;
        this.NEXT_TELEMETRY_DEBUG = NEXT_TELEMETRY_DEBUG;
        this.distDir = distDir;
        const storageDirectory = getStorageDirectory(distDir);
        try {
            // `conf` incorrectly throws a permission error during initialization
            // instead of waiting for first use. We need to handle it, otherwise the
            // process may crash.
            this.conf = new _conf.default({
                projectName: "nextjs",
                cwd: storageDirectory
            });
        } catch (_) {
            this.conf = null;
        }
        this.sessionId = (0, _crypto.randomBytes)(32).toString("hex");
        this.queue = new Set();
        this.notify();
    }
    get anonymousId() {
        const val = this.conf && this.conf.get(TELEMETRY_KEY_ID);
        if (val) {
            return val;
        }
        const generated = (0, _crypto.randomBytes)(32).toString("hex");
        this.conf && this.conf.set(TELEMETRY_KEY_ID, generated);
        return generated;
    }
    get salt() {
        const val = this.conf && this.conf.get(TELEMETRY_KEY_SALT);
        if (val) {
            return val;
        }
        const generated = (0, _crypto.randomBytes)(16).toString("hex");
        this.conf && this.conf.set(TELEMETRY_KEY_SALT, generated);
        return generated;
    }
    get isDisabled() {
        if (!!this.NEXT_TELEMETRY_DISABLED || !this.conf) {
            return true;
        }
        return this.conf.get(TELEMETRY_KEY_ENABLED, true) === false;
    }
    get isEnabled() {
        return !this.NEXT_TELEMETRY_DISABLED && !!this.conf && this.conf.get(TELEMETRY_KEY_ENABLED, true) !== false;
    }
    async getProjectId() {
        this.loadProjectId = this.loadProjectId || (0, _projectid.getRawProjectId)();
        return this.oneWayHash(await this.loadProjectId);
    }
}

//# sourceMappingURL=storage.js.map