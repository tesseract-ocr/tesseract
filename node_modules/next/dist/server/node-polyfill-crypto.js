// Polyfill crypto() in the Node.js environment
"use strict";
if (!global.crypto) {
    let webcrypto;
    Object.defineProperty(global, 'crypto', {
        enumerable: false,
        configurable: true,
        get () {
            if (!webcrypto) {
                webcrypto = require('node:crypto').webcrypto;
            }
            return webcrypto;
        },
        set (value) {
            webcrypto = value;
        }
    });
}

//# sourceMappingURL=node-polyfill-crypto.js.map