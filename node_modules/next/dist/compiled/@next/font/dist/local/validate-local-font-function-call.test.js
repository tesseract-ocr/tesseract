"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const validate_local_font_function_call_1 = require("./validate-local-font-function-call");
describe('validateLocalFontFunctionCall', () => {
    test('Not using default export', async () => {
        expect(() => (0, validate_local_font_function_call_1.validateLocalFontFunctionCall)('Named', {})).toThrowErrorMatchingInlineSnapshot(`"next/font/local has no named exports"`);
    });
    test('Missing src', async () => {
        expect(() => (0, validate_local_font_function_call_1.validateLocalFontFunctionCall)('', {})).toThrowErrorMatchingInlineSnapshot(`"Missing required \`src\` property"`);
    });
    test('Invalid file extension', async () => {
        expect(() => (0, validate_local_font_function_call_1.validateLocalFontFunctionCall)('', { src: './font/font-file.abc' })).toThrowErrorMatchingInlineSnapshot(`"Unexpected file \`./font/font-file.abc\`"`);
    });
    test('Invalid display value', async () => {
        expect(() => (0, validate_local_font_function_call_1.validateLocalFontFunctionCall)('', {
            src: './font-file.woff2',
            display: 'invalid',
        })).toThrowErrorMatchingInlineSnapshot(`
      "Invalid display value \`invalid\`.
      Available display values: \`auto\`, \`block\`, \`swap\`, \`fallback\`, \`optional\`"
    `);
    });
    test('Invalid declaration', async () => {
        expect(() => (0, validate_local_font_function_call_1.validateLocalFontFunctionCall)('', {
            src: './font-file.woff2',
            declarations: [{ prop: 'src', value: '/hello.woff2' }],
        })).toThrowErrorMatchingInlineSnapshot(`"Invalid declaration prop: \`src\`"`);
    });
    test('Empty src array', async () => {
        expect(() => (0, validate_local_font_function_call_1.validateLocalFontFunctionCall)('', {
            src: [],
        })).toThrowErrorMatchingInlineSnapshot(`"Unexpected empty \`src\` array."`);
    });
});
