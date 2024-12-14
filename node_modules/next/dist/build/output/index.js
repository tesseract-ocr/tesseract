"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ampValidation: null,
    formatAmpMessages: null,
    reportTrigger: null,
    watchCompilers: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ampValidation: function() {
        return ampValidation;
    },
    formatAmpMessages: function() {
        return formatAmpMessages;
    },
    reportTrigger: function() {
        return reportTrigger;
    },
    watchCompilers: function() {
        return watchCompilers;
    }
});
const _picocolors = require("../../lib/picocolors");
const _stripansi = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/strip-ansi"));
const _texttable = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/text-table"));
const _unistore = /*#__PURE__*/ _interop_require_default(require("next/dist/compiled/unistore"));
const _formatwebpackmessages = /*#__PURE__*/ _interop_require_default(require("../../client/components/react-dev-overlay/internal/helpers/format-webpack-messages"));
const _store = require("./store");
const _constants = require("../../shared/lib/constants");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
function formatAmpMessages(amp) {
    let output = (0, _picocolors.bold)('Amp Validation') + '\n\n';
    let messages = [];
    const chalkError = (0, _picocolors.red)('error');
    function ampError(page, error) {
        messages.push([
            page,
            chalkError,
            error.message,
            error.specUrl || ''
        ]);
    }
    const chalkWarn = (0, _picocolors.yellow)('warn');
    function ampWarn(page, warn) {
        messages.push([
            page,
            chalkWarn,
            warn.message,
            warn.specUrl || ''
        ]);
    }
    for(const page in amp){
        let { errors, warnings } = amp[page];
        const devOnlyFilter = (err)=>err.code !== 'DEV_MODE_ONLY';
        errors = errors.filter(devOnlyFilter);
        warnings = warnings.filter(devOnlyFilter);
        if (!(errors.length || warnings.length)) {
            continue;
        }
        if (errors.length) {
            ampError(page, errors[0]);
            for(let index = 1; index < errors.length; ++index){
                ampError('', errors[index]);
            }
        }
        if (warnings.length) {
            ampWarn(errors.length ? '' : page, warnings[0]);
            for(let index = 1; index < warnings.length; ++index){
                ampWarn('', warnings[index]);
            }
        }
        messages.push([
            '',
            '',
            '',
            ''
        ]);
    }
    if (!messages.length) {
        return '';
    }
    output += (0, _texttable.default)(messages, {
        align: [
            'l',
            'l',
            'l',
            'l'
        ],
        stringLength (str) {
            return (0, _stripansi.default)(str).length;
        }
    });
    return output;
}
const buildStore = (0, _unistore.default)({
    // @ts-expect-error initial value
    client: {},
    // @ts-expect-error initial value
    server: {},
    // @ts-expect-error initial value
    edgeServer: {}
});
let buildWasDone = false;
let clientWasLoading = true;
let serverWasLoading = true;
let edgeServerWasLoading = false;
buildStore.subscribe((state)=>{
    const { amp, client, server, edgeServer, trigger, url } = state;
    const { appUrl } = _store.store.getState();
    if (client.loading || server.loading || (edgeServer == null ? void 0 : edgeServer.loading)) {
        _store.store.setState({
            bootstrap: false,
            appUrl: appUrl,
            // If it takes more than 3 seconds to compile, mark it as loading status
            loading: true,
            trigger,
            url
        }, true);
        clientWasLoading = !buildWasDone && clientWasLoading || client.loading;
        serverWasLoading = !buildWasDone && serverWasLoading || server.loading;
        edgeServerWasLoading = !buildWasDone && edgeServerWasLoading || edgeServer.loading;
        buildWasDone = false;
        return;
    }
    buildWasDone = true;
    let partialState = {
        bootstrap: false,
        appUrl: appUrl,
        loading: false,
        typeChecking: false,
        totalModulesCount: (clientWasLoading ? client.totalModulesCount : 0) + (serverWasLoading ? server.totalModulesCount : 0) + (edgeServerWasLoading ? (edgeServer == null ? void 0 : edgeServer.totalModulesCount) || 0 : 0),
        hasEdgeServer: !!edgeServer
    };
    if (client.errors && clientWasLoading) {
        // Show only client errors
        _store.store.setState({
            ...partialState,
            errors: client.errors,
            warnings: null
        }, true);
    } else if (server.errors && serverWasLoading) {
        _store.store.setState({
            ...partialState,
            errors: server.errors,
            warnings: null
        }, true);
    } else if (edgeServer.errors && edgeServerWasLoading) {
        _store.store.setState({
            ...partialState,
            errors: edgeServer.errors,
            warnings: null
        }, true);
    } else {
        // Show warnings from all of them
        const warnings = [
            ...client.warnings || [],
            ...server.warnings || [],
            ...edgeServer.warnings || []
        ].concat(formatAmpMessages(amp) || []);
        _store.store.setState({
            ...partialState,
            errors: null,
            warnings: warnings.length === 0 ? null : warnings
        }, true);
    }
});
function ampValidation(page, errors, warnings) {
    const { amp } = buildStore.getState();
    if (!(errors.length || warnings.length)) {
        buildStore.setState({
            amp: Object.keys(amp).filter((k)=>k !== page).sort()// eslint-disable-next-line no-sequences
            .reduce((a, c)=>(a[c] = amp[c], a), {})
        });
        return;
    }
    const newAmp = {
        ...amp,
        [page]: {
            errors,
            warnings
        }
    };
    buildStore.setState({
        amp: Object.keys(newAmp).sort()// eslint-disable-next-line no-sequences
        .reduce((a, c)=>(a[c] = newAmp[c], a), {})
    });
}
function watchCompilers(client, server, edgeServer) {
    buildStore.setState({
        client: {
            loading: true
        },
        server: {
            loading: true
        },
        edgeServer: {
            loading: true
        },
        trigger: 'initial',
        url: undefined
    });
    function tapCompiler(key, compiler, onEvent) {
        compiler.hooks.invalid.tap(`NextJsInvalid-${key}`, ()=>{
            onEvent({
                loading: true
            });
        });
        compiler.hooks.done.tap(`NextJsDone-${key}`, (stats)=>{
            buildStore.setState({
                amp: {}
            });
            const { errors, warnings } = (0, _formatwebpackmessages.default)(stats.toJson({
                preset: 'errors-warnings',
                moduleTrace: true
            }));
            const hasErrors = !!(errors == null ? void 0 : errors.length);
            const hasWarnings = !!(warnings == null ? void 0 : warnings.length);
            onEvent({
                loading: false,
                totalModulesCount: stats.compilation.modules.size,
                errors: hasErrors ? errors : null,
                warnings: hasWarnings ? warnings : null
            });
        });
    }
    tapCompiler(_constants.COMPILER_NAMES.client, client, (status)=>{
        if (!status.loading && !buildStore.getState().server.loading && !buildStore.getState().edgeServer.loading && status.totalModulesCount > 0) {
            buildStore.setState({
                client: status,
                trigger: undefined,
                url: undefined
            });
        } else {
            buildStore.setState({
                client: status
            });
        }
    });
    tapCompiler(_constants.COMPILER_NAMES.server, server, (status)=>{
        if (!status.loading && !buildStore.getState().client.loading && !buildStore.getState().edgeServer.loading && status.totalModulesCount > 0) {
            buildStore.setState({
                server: status,
                trigger: undefined,
                url: undefined
            });
        } else {
            buildStore.setState({
                server: status
            });
        }
    });
    tapCompiler(_constants.COMPILER_NAMES.edgeServer, edgeServer, (status)=>{
        if (!status.loading && !buildStore.getState().client.loading && !buildStore.getState().server.loading && status.totalModulesCount > 0) {
            buildStore.setState({
                edgeServer: status,
                trigger: undefined,
                url: undefined
            });
        } else {
            buildStore.setState({
                edgeServer: status
            });
        }
    });
}
function reportTrigger(trigger, url) {
    buildStore.setState({
        trigger,
        url
    });
}

//# sourceMappingURL=index.js.map