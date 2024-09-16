"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.formatAvailableValues = void 0;
/**
 * Formats an array of values into a string that can be used error messages.
 * ["a", "b", "c"] => "`a`, `b`, `c`"
 */
const formatAvailableValues = (values) => values.map((val) => `\`${val}\``).join(', ');
exports.formatAvailableValues = formatAvailableValues;
