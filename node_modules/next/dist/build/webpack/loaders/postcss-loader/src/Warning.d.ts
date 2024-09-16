/**
 * **PostCSS Plugin Warning**
 *
 * Loader wrapper for postcss plugin warnings (`root.messages`)
 *
 * @class Warning
 * @extends Error
 *
 * @param {Object} warning PostCSS Warning
 */
export default class Warning extends Error {
    stack: any;
    constructor(warning: any);
}
