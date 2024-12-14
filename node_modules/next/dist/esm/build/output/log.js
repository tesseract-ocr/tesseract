import { bold, green, magenta, red, yellow, white } from '../../lib/picocolors';
import { LRUCache } from '../../server/lib/lru-cache';
export const prefixes = {
    wait: white(bold('○')),
    error: red(bold('⨯')),
    warn: yellow(bold('⚠')),
    ready: '▲',
    info: white(bold(' ')),
    event: green(bold('✓')),
    trace: magenta(bold('»'))
};
const LOGGING_METHOD = {
    log: 'log',
    warn: 'warn',
    error: 'error'
};
function prefixedLog(prefixType, ...message) {
    if ((message[0] === '' || message[0] === undefined) && message.length === 1) {
        message.shift();
    }
    const consoleMethod = prefixType in LOGGING_METHOD ? LOGGING_METHOD[prefixType] : 'log';
    const prefix = prefixes[prefixType];
    // If there's no message, don't print the prefix but a new line
    if (message.length === 0) {
        console[consoleMethod]('');
    } else {
        // Ensure if there's ANSI escape codes it's concatenated into one string.
        // Chrome DevTool can only handle color if it's in one string.
        if (message.length === 1 && typeof message[0] === 'string') {
            console[consoleMethod](' ' + prefix + ' ' + message[0]);
        } else {
            console[consoleMethod](' ' + prefix, ...message);
        }
    }
}
export function bootstrap(...message) {
    // logging format: ' <prefix> <message>'
    // e.g. ' ✓ Compiled successfully'
    // Add spaces to align with the indent of other logs
    console.log('   ' + message.join(' '));
}
export function wait(...message) {
    prefixedLog('wait', ...message);
}
export function error(...message) {
    prefixedLog('error', ...message);
}
export function warn(...message) {
    prefixedLog('warn', ...message);
}
export function ready(...message) {
    prefixedLog('ready', ...message);
}
export function info(...message) {
    prefixedLog('info', ...message);
}
export function event(...message) {
    prefixedLog('event', ...message);
}
export function trace(...message) {
    prefixedLog('trace', ...message);
}
const warnOnceCache = new LRUCache(10000, (value)=>value.length);
export function warnOnce(...message) {
    const key = message.join(' ');
    if (!warnOnceCache.has(key)) {
        warnOnceCache.set(key, key);
        warn(...message);
    }
}

//# sourceMappingURL=log.js.map