"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getOnline", {
    enumerable: true,
    get: function() {
        return getOnline;
    }
});
const _child_process = require("child_process");
const _promises = /*#__PURE__*/ _interop_require_default(require("dns/promises"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getProxy() {
    if (process.env.https_proxy) {
        return process.env.https_proxy;
    }
    try {
        const httpsProxy = (0, _child_process.execSync)("npm config get https-proxy", {
            encoding: "utf8"
        }).trim();
        return httpsProxy !== "null" ? httpsProxy : undefined;
    } catch (e) {
        return;
    }
}
async function getOnline() {
    try {
        await _promises.default.lookup("registry.yarnpkg.com");
        return true;
    } catch  {
        const proxy = getProxy();
        if (!proxy) {
            return false;
        }
        try {
            const { hostname } = new URL(proxy);
            await _promises.default.lookup(hostname);
            return true;
        } catch  {
            return false;
        }
    }
}

//# sourceMappingURL=get-online.js.map