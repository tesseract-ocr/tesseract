"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "createProxyServer", {
    enumerable: true,
    get: function() {
        return createProxyServer;
    }
});
const _http = /*#__PURE__*/ _interop_require_default(require("http"));
const _types = require("./types");
const _fetchapi = require("./fetch-api");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
async function readBody(req) {
    const acc = [];
    for await (const chunk of req){
        acc.push(chunk);
    }
    return Buffer.concat(acc);
}
async function createProxyServer({ onFetch }) {
    const server = _http.default.createServer(async (req, res)=>{
        if (req.url !== "/") {
            res.writeHead(404);
            res.end();
            return;
        }
        let json;
        try {
            json = JSON.parse((await readBody(req)).toString("utf-8"));
        } catch (e) {
            res.writeHead(400);
            res.end();
            return;
        }
        const { api } = json;
        let response;
        switch(api){
            case "fetch":
                if (onFetch) {
                    response = await (0, _fetchapi.handleFetch)(json, onFetch);
                }
                break;
            default:
                break;
        }
        if (!response) {
            response = _types.UNHANDLED;
        }
        res.writeHead(200, {
            "Content-Type": "application/json"
        });
        res.write(JSON.stringify(response));
        res.end();
    });
    await new Promise((resolve)=>{
        server.listen(0, "localhost", ()=>{
            resolve(undefined);
        });
    });
    const address = server.address();
    if (!address || typeof address !== "object") {
        server.close();
        throw new Error("Failed to create a proxy server");
    }
    const port = address.port;
    const fetchWith = (input, init, testData)=>{
        const request = new Request(input, init);
        request.headers.set("Next-Test-Proxy-Port", String(port));
        request.headers.set("Next-Test-Data", testData ?? "");
        return fetch(request);
    };
    return {
        port,
        close: ()=>server.close(),
        fetchWith
    };
}

//# sourceMappingURL=server.js.map