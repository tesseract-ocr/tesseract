"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getFreePort", {
    enumerable: true,
    get: function() {
        return getFreePort;
    }
});
const _http = /*#__PURE__*/ _interop_require_default(require("http"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const getFreePort = async ()=>{
    return new Promise((resolve, reject)=>{
        const server = _http.default.createServer(()=>{});
        server.listen(0, ()=>{
            const address = server.address();
            server.close();
            if (address && typeof address === "object") {
                resolve(address.port);
            } else {
                reject(new Error("invalid address from server: " + (address == null ? void 0 : address.toString())));
            }
        });
    });
};

//# sourceMappingURL=worker-utils.js.map