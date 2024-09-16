"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "formatHostname", {
    enumerable: true,
    get: function() {
        return formatHostname;
    }
});
const _isipv6 = require("./is-ipv6");
function formatHostname(hostname) {
    return (0, _isipv6.isIPv6)(hostname) ? `[${hostname}]` : hostname;
}

//# sourceMappingURL=format-hostname.js.map