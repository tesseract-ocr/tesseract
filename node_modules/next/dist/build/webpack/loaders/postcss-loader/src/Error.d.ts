/**
 * **PostCSS Syntax Error**
 *
 * Loader wrapper for postcss syntax errors
 *
 * @class SyntaxError
 * @extends Error
 *
 * @param {Object} err CssSyntaxError
 */
export default class PostCSSSyntaxError extends Error {
    stack: any;
    constructor(error: any);
}
