import { red } from "./picocolors";
import { Worker } from "next/dist/compiled/jest-worker";
import { existsSync } from "fs";
import { join } from "path";
import { ESLINT_DEFAULT_DIRS } from "./constants";
import { eventLintCheckCompleted } from "../telemetry/events";
import { CompileError } from "./compile-error";
import isError from "./is-error";
export async function verifyAndLint(dir, cacheLocation, configLintDirs, enableWorkerThreads, telemetry) {
    try {
        const lintWorkers = new Worker(require.resolve("./eslint/runLintCheck"), {
            numWorkers: 1,
            enableWorkerThreads,
            maxRetries: 0
        });
        lintWorkers.getStdout().pipe(process.stdout);
        lintWorkers.getStderr().pipe(process.stderr);
        const lintDirs = (configLintDirs ?? ESLINT_DEFAULT_DIRS).reduce((res, d)=>{
            const currDir = join(dir, d);
            if (!existsSync(currDir)) return res;
            res.push(currDir);
            return res;
        }, []);
        const lintResults = await lintWorkers.runLintCheck(dir, lintDirs, {
            lintDuringBuild: true,
            eslintOptions: {
                cacheLocation
            }
        });
        const lintOutput = typeof lintResults === "string" ? lintResults : lintResults == null ? void 0 : lintResults.output;
        if (typeof lintResults !== "string" && (lintResults == null ? void 0 : lintResults.eventInfo)) {
            telemetry.record(eventLintCheckCompleted({
                ...lintResults.eventInfo,
                buildLint: true
            }));
        }
        if (typeof lintResults !== "string" && (lintResults == null ? void 0 : lintResults.isError) && lintOutput) {
            await telemetry.flush();
            throw new CompileError(lintOutput);
        }
        if (lintOutput) {
            console.log(lintOutput);
        }
        lintWorkers.end();
    } catch (err) {
        if (isError(err)) {
            if (err.type === "CompileError" || err instanceof CompileError) {
                console.error(red("\nFailed to compile."));
                console.error(err.message);
                process.exit(1);
            } else if (err.type === "FatalError") {
                console.error(err.message);
                process.exit(1);
            }
        }
        throw err;
    }
}

//# sourceMappingURL=verifyAndLint.js.map