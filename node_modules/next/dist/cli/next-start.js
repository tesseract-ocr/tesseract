#!/usr/bin/env node
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "nextStart", {
    enumerable: true,
    get: function() {
        return nextStart;
    }
});
require("../server/lib/cpu-profile");
const _startserver = require("../server/lib/start-server");
const _utils = require("../server/lib/utils");
const _getprojectdir = require("../lib/get-project-dir");
const _getreservedport = require("../lib/helpers/get-reserved-port");
/**
 * Start the Next.js server
 *
 * @param options The options for the start command
 * @param directory The directory to start the server in
 */ const nextStart = async (options, directory)=>{
    const dir = (0, _getprojectdir.getProjectDir)(directory);
    const hostname = options.hostname;
    const port = options.port;
    const keepAliveTimeout = options.keepAliveTimeout;
    if ((0, _getreservedport.isPortIsReserved)(port)) {
        (0, _utils.printAndExit)((0, _getreservedport.getReservedPortExplanation)(port), 1);
    }
    await (0, _startserver.startServer)({
        dir,
        isDev: false,
        hostname,
        port,
        keepAliveTimeout
    });
};

//# sourceMappingURL=next-start.js.map