'use client';
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "ClientPageRoot", {
    enumerable: true,
    get: function() {
        return ClientPageRoot;
    }
});
const _jsxruntime = require("react/jsx-runtime");
const _invarianterror = require("../../shared/lib/invariant-error");
function ClientPageRoot(param) {
    let { Component, searchParams, params, // eslint-disable-next-line @typescript-eslint/no-unused-vars
    promises } = param;
    if (typeof window === 'undefined') {
        const { workAsyncStorage } = require('../../server/app-render/work-async-storage.external');
        let clientSearchParams;
        let clientParams;
        // We are going to instrument the searchParams prop with tracking for the
        // appropriate context. We wrap differently in prerendering vs rendering
        const store = workAsyncStorage.getStore();
        if (!store) {
            throw new _invarianterror.InvariantError('Expected workStore to exist when handling searchParams in a client Page.');
        }
        const { createSearchParamsFromClient } = require('../../server/request/search-params');
        clientSearchParams = createSearchParamsFromClient(searchParams, store);
        const { createParamsFromClient } = require('../../server/request/params');
        clientParams = createParamsFromClient(params, store);
        return /*#__PURE__*/ (0, _jsxruntime.jsx)(Component, {
            params: clientParams,
            searchParams: clientSearchParams
        });
    } else {
        const { createRenderSearchParamsFromClient } = require('../../server/request/search-params.browser');
        const clientSearchParams = createRenderSearchParamsFromClient(searchParams);
        const { createRenderParamsFromClient } = require('../../server/request/params.browser');
        const clientParams = createRenderParamsFromClient(params);
        return /*#__PURE__*/ (0, _jsxruntime.jsx)(Component, {
            params: clientParams,
            searchParams: clientSearchParams
        });
    }
}

if ((typeof exports.default === 'function' || (typeof exports.default === 'object' && exports.default !== null)) && typeof exports.default.__esModule === 'undefined') {
  Object.defineProperty(exports.default, '__esModule', { value: true });
  Object.assign(exports.default, exports);
  module.exports = exports.default;
}

//# sourceMappingURL=client-page.js.map