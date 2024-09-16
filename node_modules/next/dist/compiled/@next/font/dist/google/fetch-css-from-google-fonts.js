"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.fetchCSSFromGoogleFonts = void 0;
// @ts-ignore
const node_fetch_1 = __importDefault(require("next/dist/compiled/node-fetch"));
const next_font_error_1 = require("../next-font-error");
const get_proxy_agent_1 = require("./get-proxy-agent");
const retry_1 = require("./retry");
/**
 * Fetches the CSS containing the @font-face declarations from Google Fonts.
 * The fetch has a user agent header with a modern browser to ensure we'll get .woff2 files.
 *
 * The env variable NEXT_FONT_GOOGLE_MOCKED_RESPONSES may be set containing a path to mocked data.
 * It's used to define mocked data to avoid hitting the Google Fonts API during tests.
 */
async function fetchCSSFromGoogleFonts(url, fontFamily, isDev) {
    // Check if mocked responses are defined, if so use them instead of fetching from Google Fonts
    let mockedResponse;
    if (process.env.NEXT_FONT_GOOGLE_MOCKED_RESPONSES) {
        const mockFile = require(process.env.NEXT_FONT_GOOGLE_MOCKED_RESPONSES);
        mockedResponse = mockFile[url];
        if (!mockedResponse) {
            (0, next_font_error_1.nextFontError)('Missing mocked response for URL: ' + url);
        }
    }
    let cssResponse;
    if (mockedResponse) {
        // Just use the mocked CSS if it's set
        cssResponse = mockedResponse;
    }
    else {
        // Retry the fetch a few times in case of network issues as some font files
        // are quite large:
        // https://github.com/vercel/next.js/issues/45080
        cssResponse = await (0, retry_1.retry)(async () => {
            const controller = isDev && typeof AbortController !== 'undefined'
                ? new AbortController()
                : undefined;
            const signal = controller === null || controller === void 0 ? void 0 : controller.signal;
            const timeoutId = controller
                ? setTimeout(() => controller.abort(), 3000)
                : undefined;
            const res = await (0, node_fetch_1.default)(url, {
                agent: (0, get_proxy_agent_1.getProxyAgent)(),
                // Add a timeout in dev
                signal,
                headers: {
                    // The file format is based off of the user agent, make sure woff2 files are fetched
                    'user-agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/104.0.0.0 Safari/537.36',
                },
            }).finally(() => {
                timeoutId && clearTimeout(timeoutId);
            });
            if (!res.ok) {
                (0, next_font_error_1.nextFontError)(`Failed to fetch font \`${fontFamily}\`.\nURL: ${url}\n\nPlease check if the network is available.`);
            }
            return res.text();
        }, 3);
    }
    return cssResponse;
}
exports.fetchCSSFromGoogleFonts = fetchCSSFromGoogleFonts;
