"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.fetchFontFile = void 0;
// @ts-ignore
const node_fetch_1 = __importDefault(require("next/dist/compiled/node-fetch"));
const get_proxy_agent_1 = require("./get-proxy-agent");
const retry_1 = require("./retry");
/**
 * Fetch the url and return a buffer with the font file.
 */
async function fetchFontFile(url, isDev) {
    // Check if we're using mocked data
    if (process.env.NEXT_FONT_GOOGLE_MOCKED_RESPONSES) {
        // If it's an absolute path, read the file from the filesystem
        if (url.startsWith('/')) {
            return require('fs').readFileSync(url);
        }
        // Otherwise just return a unique buffer
        return Buffer.from(url);
    }
    return await (0, retry_1.retry)(async () => {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), 3000);
        const arrayBuffer = await (0, node_fetch_1.default)(url, {
            agent: (0, get_proxy_agent_1.getProxyAgent)(),
            // Add a timeout in dev
            signal: isDev ? controller.signal : undefined,
        })
            .then((r) => r.arrayBuffer())
            .finally(() => {
            clearTimeout(timeoutId);
        });
        return Buffer.from(arrayBuffer);
    }, 3);
}
exports.fetchFontFile = fetchFontFile;
