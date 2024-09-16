"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const pick_font_file_for_fallback_generation_1 = require("./pick-font-file-for-fallback-generation");
describe('pickFontFileForFallbackGeneration', () => {
    it('should pick the weight closest to 400', () => {
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            {
                weight: '300',
            },
            {
                weight: '600',
            },
        ])).toEqual({
            weight: '300',
        });
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            { weight: '200' },
            {
                weight: '500',
            },
        ])).toEqual({
            weight: '500',
        });
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            {
                weight: 'normal',
            },
            {
                weight: '700',
            },
        ])).toEqual({
            weight: 'normal',
        });
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            {
                weight: 'bold',
            },
            {
                weight: '900',
            },
        ])).toEqual({
            weight: 'bold',
        });
    });
    it('should pick the thinner weight if both have the same distance to 400', () => {
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            {
                weight: '300',
            },
            {
                weight: '500',
            },
        ])).toEqual({
            weight: '300',
        });
    });
    it('should pick variable range closest to 400', () => {
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            {
                weight: '100 300',
            },
            {
                weight: '600 900',
            },
        ])).toEqual({
            weight: '100 300',
        });
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            { weight: '100 200' },
            {
                weight: '500 800',
            },
        ])).toEqual({
            weight: '500 800',
        });
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            { weight: '100 900' },
            {
                weight: '300 399',
            },
        ])).toEqual({
            weight: '100 900',
        });
    });
    it('should prefer normal style over italic', () => {
        expect((0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            { weight: '400', style: 'normal' },
            { weight: '400', style: 'italic' },
        ])).toEqual({ weight: '400', style: 'normal' });
    });
    it('should error on invalid weight in array', async () => {
        expect(() => (0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            { path: './font1.woff2', weight: 'normal bold' },
            { path: './font2.woff2', weight: '400 bold' },
            { path: './font3.woff2', weight: 'normal 700' },
            { path: './font4.woff2', weight: '100 abc' },
        ])).toThrowErrorMatchingInlineSnapshot(`
      "Invalid weight value in src array: \`100 abc\`.
      Expected \`normal\`, \`bold\` or a number."
    `);
    });
    test('Invalid variable weight in array', async () => {
        expect(() => (0, pick_font_file_for_fallback_generation_1.pickFontFileForFallbackGeneration)([
            { path: './font1.woff2', weight: 'normal bold' },
            { path: './font2.woff2', weight: '400 bold' },
            { path: './font3.woff2', weight: 'normal 700' },
            { path: './font4.woff2', weight: '100 abc' },
        ])).toThrowErrorMatchingInlineSnapshot(`
      "Invalid weight value in src array: \`100 abc\`.
      Expected \`normal\`, \`bold\` or a number."
    `);
    });
});
