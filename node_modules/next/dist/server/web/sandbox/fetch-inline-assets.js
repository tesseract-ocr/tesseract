"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "fetchInlineAsset", {
    enumerable: true,
    get: function() {
        return fetchInlineAsset;
    }
});
const _fs = require("fs");
const _bodystreams = require("../../body-streams");
const _path = require("path");
async function fetchInlineAsset(options) {
    const inputString = String(options.input);
    if (!inputString.startsWith("blob:")) {
        return;
    }
    const name = inputString.replace("blob:", "");
    const asset = options.assets ? options.assets.find((x)=>x.name === name) : {
        name,
        filePath: name
    };
    if (!asset) {
        return;
    }
    const filePath = (0, _path.resolve)(options.distDir, asset.filePath);
    const fileIsReadable = await _fs.promises.access(filePath).then(()=>true, ()=>false);
    if (fileIsReadable) {
        const readStream = (0, _fs.createReadStream)(filePath);
        return new options.context.Response((0, _bodystreams.requestToBodyStream)(options.context, Uint8Array, readStream));
    }
}

//# sourceMappingURL=fetch-inline-assets.js.map