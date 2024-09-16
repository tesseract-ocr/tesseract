"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const get_font_axes_1 = require("./get-font-axes");
describe('getFontAxes errors', () => {
    test('Setting axes on font without definable axes', () => {
        expect(() => (0, get_font_axes_1.getFontAxes)('Lora', ['variable'], [], [])).toThrowErrorMatchingInlineSnapshot(`"Font \`Lora\` has no definable \`axes\`"`);
    });
    test('Invalid axes value', async () => {
        expect(() => (0, get_font_axes_1.getFontAxes)('Inter', ['variable'], [], true))
            .toThrowErrorMatchingInlineSnapshot(`
      "Invalid axes value for font \`Inter\`, expected an array of axes.
      Available axes: \`opsz\`"
    `);
    });
    test('Invalid value in axes array', async () => {
        expect(() => (0, get_font_axes_1.getFontAxes)('Roboto Flex', ['variable'], [], ['INVALID']))
            .toThrowErrorMatchingInlineSnapshot(`
      "Invalid axes value \`INVALID\` for font \`Roboto Flex\`.
      Available axes: \`GRAD\`, \`XOPQ\`, \`XTRA\`, \`YOPQ\`, \`YTAS\`, \`YTDE\`, \`YTFI\`, \`YTLC\`, \`YTUC\`, \`opsz\`, \`slnt\`, \`wdth\`"
    `);
    });
});
