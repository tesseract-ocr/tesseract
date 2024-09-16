"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getRawProjectId", {
    enumerable: true,
    get: function() {
        return getRawProjectId;
    }
});
const _child_process = require("child_process");
// Q: Why does Next.js need a project ID? Why is it looking at my git remote?
// A:
// Next.js' telemetry is and always will be completely anonymous. Because of
// this, we need a way to differentiate different projects to track feature
// usage accurately. For example, to prevent a feature from appearing to be
// constantly `used` and then `unused` when switching between local projects.
// To reiterate,
// we **never** can read your actual git remote. The value is hashed one-way
// with random salt data, making it impossible for us to reverse or try to
// guess the remote by re-computing hashes.
async function _getProjectIdByGit() {
    try {
        let resolve, reject;
        const promise = new Promise((res, rej)=>{
            resolve = res;
            reject = rej;
        });
        (0, _child_process.exec)(`git config --local --get remote.origin.url`, {
            timeout: 1000,
            windowsHide: true
        }, (error, stdout)=>{
            if (error) {
                reject(error);
                return;
            }
            resolve(stdout);
        });
        return String(await promise).trim();
    } catch (_) {
        return null;
    }
}
async function getRawProjectId() {
    return await _getProjectIdByGit() || process.env.REPOSITORY_URL || process.cwd();
}

//# sourceMappingURL=project-id.js.map