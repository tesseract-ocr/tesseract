"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "tcpProxy", {
    enumerable: true,
    get: function() {
        return tcpProxy;
    }
});
const _net = /*#__PURE__*/ _interop_require_default(require("net"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function tcpProxy(addresses) {
    // net.Socket docs https://nodejs.org/api/net.html#class-netsocket
    const originalConnect = _net.default.Socket.prototype.connect;
    // Override the connect method
    _net.default.Socket.prototype.connect = function(...args) {
        // First, check if the first argument is an object and not null
        if (typeof args[0] === "object" && args[0] !== null) {
            const options = args[0];
            // check if the options has what we need
            if ("port" in options && options.port !== undefined && "host" in options && options.host !== undefined) {
                addresses.push({
                    addr: options.host,
                    port: options.port.toString()
                });
            }
        }
        return originalConnect.apply(this, args);
    };
    return ()=>{
        // Restore the original connect method
        _net.default.Socket.prototype.connect = originalConnect;
    };
}

//# sourceMappingURL=tcp.js.map