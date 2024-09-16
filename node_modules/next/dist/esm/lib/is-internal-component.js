export function isInternalComponent(pathname) {
    switch(pathname){
        case "next/dist/pages/_app":
        case "next/dist/pages/_document":
            return true;
        default:
            return false;
    }
}
export function isNonRoutePagesPage(pathname) {
    return pathname === "/_app" || pathname === "/_document";
}

//# sourceMappingURL=is-internal-component.js.map