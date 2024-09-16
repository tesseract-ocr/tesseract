// Override chunk URL mapping in the webpack runtime
// https://github.com/webpack/webpack/blob/2738eebc7880835d88c727d364ad37f3ec557593/lib/RuntimeGlobals.js#L204
import { getDeploymentIdQueryOrEmptyString } from "../build/deployment-id";
// If we have a deployment ID, we need to append it to the webpack chunk names
// I am keeping the process check explicit so this can be statically optimized
if (process.env.NEXT_DEPLOYMENT_ID) {
    const suffix = getDeploymentIdQueryOrEmptyString();
    // eslint-disable-next-line no-undef
    const getChunkScriptFilename = __webpack_require__.u;
    // eslint-disable-next-line no-undef
    __webpack_require__.u = function() {
        for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
            args[_key] = arguments[_key];
        }
        return(// We enode the chunk filename because our static server matches against and encoded
        // filename path.
        encodeURI(getChunkScriptFilename(...args) + suffix));
    };
    // eslint-disable-next-line no-undef
    const getChunkCssFilename = __webpack_require__.k;
    // eslint-disable-next-line no-undef
    __webpack_require__.k = function() {
        for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
            args[_key] = arguments[_key];
        }
        return getChunkCssFilename(...args) + suffix;
    };
    // eslint-disable-next-line no-undef
    const getMiniCssFilename = __webpack_require__.miniCssF;
    // eslint-disable-next-line no-undef
    __webpack_require__.miniCssF = function() {
        for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
            args[_key] = arguments[_key];
        }
        return getMiniCssFilename(...args) + suffix;
    };
} else {
    // eslint-disable-next-line no-undef
    const getChunkScriptFilename = __webpack_require__.u;
    // eslint-disable-next-line no-undef
    __webpack_require__.u = function() {
        for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
            args[_key] = arguments[_key];
        }
        return(// We enode the chunk filename because our static server matches against and encoded
        // filename path.
        encodeURI(getChunkScriptFilename(...args)));
    };
// We don't need to override __webpack_require__.k because we don't modify
// the css chunk name when not using deployment id suffixes
// WE don't need to override __webpack_require__.miniCssF because we don't modify
// the mini css chunk name when not using deployment id suffixes
}

//# sourceMappingURL=app-webpack.js.map