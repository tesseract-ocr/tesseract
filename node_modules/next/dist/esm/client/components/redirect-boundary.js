'use client';
import { jsx as _jsx } from "react/jsx-runtime";
import React, { useEffect } from 'react';
import { useRouter } from './navigation';
import { getRedirectTypeFromError, getURLFromRedirectError } from './redirect';
import { RedirectType, isRedirectError } from './redirect-error';
function HandleRedirect(param) {
    let { redirect, reset, redirectType } = param;
    const router = useRouter();
    useEffect(()=>{
        React.startTransition(()=>{
            if (redirectType === RedirectType.push) {
                router.push(redirect, {});
            } else {
                router.replace(redirect, {});
            }
            reset();
        });
    }, [
        redirect,
        redirectType,
        reset,
        router
    ]);
    return null;
}
export class RedirectErrorBoundary extends React.Component {
    static getDerivedStateFromError(error) {
        if (isRedirectError(error)) {
            const url = getURLFromRedirectError(error);
            const redirectType = getRedirectTypeFromError(error);
            return {
                redirect: url,
                redirectType
            };
        }
        // Re-throw if error is not for redirect
        throw error;
    }
    // Explicit type is needed to avoid the generated `.d.ts` having a wide return type that could be specific to the `@types/react` version.
    render() {
        const { redirect, redirectType } = this.state;
        if (redirect !== null && redirectType !== null) {
            return /*#__PURE__*/ _jsx(HandleRedirect, {
                redirect: redirect,
                redirectType: redirectType,
                reset: ()=>this.setState({
                        redirect: null
                    })
            });
        }
        return this.props.children;
    }
    constructor(props){
        super(props);
        this.state = {
            redirect: null,
            redirectType: null
        };
    }
}
export function RedirectBoundary(param) {
    let { children } = param;
    const router = useRouter();
    return /*#__PURE__*/ _jsx(RedirectErrorBoundary, {
        router: router,
        children: children
    });
}

//# sourceMappingURL=redirect-boundary.js.map