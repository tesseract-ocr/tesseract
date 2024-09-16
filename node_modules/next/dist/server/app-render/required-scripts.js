"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getRequiredScripts", {
    enumerable: true,
    get: function() {
        return getRequiredScripts;
    }
});
const _encodeuripath = require("../../shared/lib/encode-uri-path");
const _reactdom = /*#__PURE__*/ _interop_require_default(require("react-dom"));
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function getRequiredScripts(buildManifest, assetPrefix, crossOrigin, SRIManifest, qs, nonce) {
    let preinitScripts;
    let preinitScriptCommands = [];
    const bootstrapScript = {
        src: "",
        crossOrigin
    };
    const files = buildManifest.rootMainFiles.map(_encodeuripath.encodeURIPath);
    if (files.length === 0) {
        throw new Error("Invariant: missing bootstrap script. This is a bug in Next.js");
    }
    if (SRIManifest) {
        bootstrapScript.src = `${assetPrefix}/_next/` + files[0] + qs;
        bootstrapScript.integrity = SRIManifest[files[0]];
        for(let i = 1; i < files.length; i++){
            const src = `${assetPrefix}/_next/` + files[i] + qs;
            const integrity = SRIManifest[files[i]];
            preinitScriptCommands.push(src, integrity);
        }
        preinitScripts = ()=>{
            // preinitScriptCommands is a double indexed array of src/integrity pairs
            for(let i = 0; i < preinitScriptCommands.length; i += 2){
                _reactdom.default.preinit(preinitScriptCommands[i], {
                    as: "script",
                    integrity: preinitScriptCommands[i + 1],
                    crossOrigin,
                    nonce
                });
            }
        };
    } else {
        bootstrapScript.src = `${assetPrefix}/_next/` + files[0] + qs;
        for(let i = 1; i < files.length; i++){
            const src = `${assetPrefix}/_next/` + files[i] + qs;
            preinitScriptCommands.push(src);
        }
        preinitScripts = ()=>{
            // preinitScriptCommands is a singled indexed array of src values
            for(let i = 0; i < preinitScriptCommands.length; i++){
                _reactdom.default.preinit(preinitScriptCommands[i], {
                    as: "script",
                    nonce,
                    crossOrigin
                });
            }
        };
    }
    return [
        preinitScripts,
        bootstrapScript
    ];
}

//# sourceMappingURL=required-scripts.js.map