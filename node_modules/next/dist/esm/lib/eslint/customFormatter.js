import { bold, cyan, gray, red, yellow } from "../picocolors";
import path from "path";
export var MessageSeverity;
(function(MessageSeverity) {
    MessageSeverity[MessageSeverity["Warning"] = 1] = "Warning";
    MessageSeverity[MessageSeverity["Error"] = 2] = "Error";
})(MessageSeverity || (MessageSeverity = {}));
function pluginCount(messages) {
    let nextPluginWarningCount = 0;
    let nextPluginErrorCount = 0;
    for(let i = 0; i < messages.length; i++){
        const { severity, ruleId } = messages[i];
        if (ruleId == null ? void 0 : ruleId.includes("@next/next")) {
            if (severity === 1) {
                nextPluginWarningCount += 1;
            } else {
                nextPluginErrorCount += 1;
            }
        }
    }
    return {
        nextPluginErrorCount,
        nextPluginWarningCount
    };
}
function formatMessage(dir, messages, filePath) {
    let fileName = path.posix.normalize(path.relative(dir, filePath).replace(/\\/g, "/"));
    if (!fileName.startsWith(".")) {
        fileName = "./" + fileName;
    }
    let output = "\n" + cyan(fileName);
    for(let i = 0; i < messages.length; i++){
        const { message, severity, line, column, ruleId } = messages[i];
        output = output + "\n";
        if (line && column) {
            output = output + yellow(line.toString()) + ":" + yellow(column.toString()) + "  ";
        }
        if (severity === 1) {
            output += yellow(bold("Warning")) + ": ";
        } else {
            output += red(bold("Error")) + ": ";
        }
        output += message;
        if (ruleId) {
            output += "  " + gray(bold(ruleId));
        }
    }
    return output;
}
export function formatResults(baseDir, results, format) {
    let totalNextPluginErrorCount = 0;
    let totalNextPluginWarningCount = 0;
    let resultsWithMessages = results.filter(({ messages })=>messages == null ? void 0 : messages.length);
    // Track number of Next.js plugin errors and warnings
    resultsWithMessages.forEach(({ messages })=>{
        const res = pluginCount(messages);
        totalNextPluginErrorCount += res.nextPluginErrorCount;
        totalNextPluginWarningCount += res.nextPluginWarningCount;
    });
    // Use user defined formatter or Next.js's built-in custom formatter
    const output = format ? format(resultsWithMessages) : resultsWithMessages.map(({ messages, filePath })=>formatMessage(baseDir, messages, filePath)).join("\n");
    return {
        output: output,
        outputWithMessages: resultsWithMessages.length > 0 ? output + `\n\n${cyan("info")}  - Need to disable some ESLint rules? Learn more here: https://nextjs.org/docs/basic-features/eslint#disabling-rules` : "",
        totalNextPluginErrorCount,
        totalNextPluginWarningCount
    };
}

//# sourceMappingURL=customFormatter.js.map