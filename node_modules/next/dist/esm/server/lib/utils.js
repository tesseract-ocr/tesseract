import { InvalidArgumentError } from "next/dist/compiled/commander";
export function printAndExit(message, code = 1) {
    if (code === 0) {
        console.log(message);
    } else {
        console.error(message);
    }
    process.exit(code);
}
export const getDebugPort = ()=>{
    var _process_execArgv_find, _process_env_NODE_OPTIONS_match, _process_env_NODE_OPTIONS_match1, _process_env_NODE_OPTIONS;
    const debugPortStr = ((_process_execArgv_find = process.execArgv.find((localArg)=>localArg.startsWith("--inspect") || localArg.startsWith("--inspect-brk"))) == null ? void 0 : _process_execArgv_find.split("=", 2)[1]) ?? ((_process_env_NODE_OPTIONS = process.env.NODE_OPTIONS) == null ? void 0 : (_process_env_NODE_OPTIONS_match1 = _process_env_NODE_OPTIONS.match) == null ? void 0 : (_process_env_NODE_OPTIONS_match = _process_env_NODE_OPTIONS_match1.call(_process_env_NODE_OPTIONS, /--inspect(-brk)?(=(\S+))?( |$)/)) == null ? void 0 : _process_env_NODE_OPTIONS_match[3]);
    return debugPortStr ? parseInt(debugPortStr, 10) : 9229;
};
const NODE_INSPECT_RE = /--inspect(-brk)?(=\S+)?( |$)/;
export function getNodeOptionsWithoutInspect() {
    return (process.env.NODE_OPTIONS || "").replace(NODE_INSPECT_RE, "");
}
export function myParseInt(value) {
    // parseInt takes a string and a radix
    const parsedValue = parseInt(value, 10);
    if (isNaN(parsedValue) || !isFinite(parsedValue) || parsedValue < 0) {
        throw new InvalidArgumentError(`'${value}' is not a non-negative number.`);
    }
    return parsedValue;
}
export const RESTART_EXIT_CODE = 77;
export function checkNodeDebugType() {
    var _process_env_NODE_OPTIONS_match, _process_env_NODE_OPTIONS, _process_env_NODE_OPTIONS_match1, _process_env_NODE_OPTIONS1;
    let nodeDebugType = undefined;
    if (process.execArgv.some((localArg)=>localArg.startsWith("--inspect")) || ((_process_env_NODE_OPTIONS = process.env.NODE_OPTIONS) == null ? void 0 : (_process_env_NODE_OPTIONS_match = _process_env_NODE_OPTIONS.match) == null ? void 0 : _process_env_NODE_OPTIONS_match.call(_process_env_NODE_OPTIONS, /--inspect(=\S+)?( |$)/))) {
        nodeDebugType = "inspect";
    }
    if (process.execArgv.some((localArg)=>localArg.startsWith("--inspect-brk")) || ((_process_env_NODE_OPTIONS1 = process.env.NODE_OPTIONS) == null ? void 0 : (_process_env_NODE_OPTIONS_match1 = _process_env_NODE_OPTIONS1.match) == null ? void 0 : _process_env_NODE_OPTIONS_match1.call(_process_env_NODE_OPTIONS1, /--inspect-brk(=\S+)?( |$)/))) {
        nodeDebugType = "inspect-brk";
    }
    return nodeDebugType;
}
export function getMaxOldSpaceSize() {
    var _process_env_NODE_OPTIONS_match, _process_env_NODE_OPTIONS;
    const maxOldSpaceSize = (_process_env_NODE_OPTIONS = process.env.NODE_OPTIONS) == null ? void 0 : (_process_env_NODE_OPTIONS_match = _process_env_NODE_OPTIONS.match(/--max[-_]old[-_]space[-_]size=(\d+)/)) == null ? void 0 : _process_env_NODE_OPTIONS_match[1];
    return maxOldSpaceSize ? parseInt(maxOldSpaceSize, 10) : undefined;
}

//# sourceMappingURL=utils.js.map