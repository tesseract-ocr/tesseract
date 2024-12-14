'use client';
import { jsx as _jsx } from "react/jsx-runtime";
import { InvariantError } from '../../shared/lib/invariant-error';
/**
 * When the Page is a client component we send the params and searchParams to this client wrapper
 * where they are turned into dynamically tracked values before being passed to the actual Page component.
 *
 * additionally we may send promises representing the params and searchParams. We don't ever use these passed
 * values but it can be necessary for the sender to send a Promise that doesn't resolve in certain situations.
 * It is up to the caller to decide if the promises are needed.
 */ export function ClientPageRoot(param) {
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
            throw new InvariantError('Expected workStore to exist when handling searchParams in a client Page.');
        }
        const { createSearchParamsFromClient } = require('../../server/request/search-params');
        clientSearchParams = createSearchParamsFromClient(searchParams, store);
        const { createParamsFromClient } = require('../../server/request/params');
        clientParams = createParamsFromClient(params, store);
        return /*#__PURE__*/ _jsx(Component, {
            params: clientParams,
            searchParams: clientSearchParams
        });
    } else {
        const { createRenderSearchParamsFromClient } = require('../../server/request/search-params.browser');
        const clientSearchParams = createRenderSearchParamsFromClient(searchParams);
        const { createRenderParamsFromClient } = require('../../server/request/params.browser');
        const clientParams = createRenderParamsFromClient(params);
        return /*#__PURE__*/ _jsx(Component, {
            params: clientParams,
            searchParams: clientSearchParams
        });
    }
}

//# sourceMappingURL=client-page.js.map