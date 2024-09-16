"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.getProxyAgent = void 0;
// @ts-ignore
const https_proxy_agent_1 = __importDefault(require("next/dist/compiled/https-proxy-agent"));
// @ts-ignore
const http_proxy_agent_1 = __importDefault(require("next/dist/compiled/http-proxy-agent"));
/**
 * If the http(s)_proxy environment variables is set, return a proxy agent.
 */
function getProxyAgent() {
    const httpsProxy = process.env['https_proxy'] || process.env['HTTPS_PROXY'];
    if (httpsProxy) {
        return new https_proxy_agent_1.default(httpsProxy);
    }
    const httpProxy = process.env['http_proxy'] || process.env['HTTP_PROXY'];
    if (httpProxy) {
        return new http_proxy_agent_1.default(httpProxy);
    }
}
exports.getProxyAgent = getProxyAgent;
