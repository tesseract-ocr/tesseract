"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "nextExport", {
    enumerable: true,
    get: function() {
        return nextExport;
    }
});
const _picocolors = require("../lib/picocolors");
const _log = require("../build/output/log");
const nextExport = ()=>{
    (0, _log.error)(`
    \`next export\` has been removed in favor of 'output: export' in next.config.js.\nLearn more: ${(0, _picocolors.cyan)('https://nextjs.org/docs/app/building-your-application/deploying/static-exports')}
  `);
    process.exit(1);
};

//# sourceMappingURL=next-export.js.map