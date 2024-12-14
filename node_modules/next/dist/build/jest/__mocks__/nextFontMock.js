"use strict";
module.exports = new Proxy({}, {
    get: function getter() {
        return ()=>({
                className: 'className',
                variable: 'variable',
                style: {
                    fontFamily: 'fontFamily'
                }
            });
    }
});

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=nextFontMock.js.map