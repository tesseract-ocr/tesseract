"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.retry = retry;
// eslint-disable-next-line import/no-extraneous-dependencies
// @ts-expect-error File exists
const async_retry_1 = __importDefault(require("next/dist/compiled/async-retry"));
async function retry(fn, retries) {
    return await (0, async_retry_1.default)(fn, {
        retries,
        onRetry(e, attempt) {
            console.error(e.message + `\n\nRetrying ${attempt}/${retries}...`);
        },
        minTimeout: 100,
    });
}
