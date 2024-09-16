// This has to be a shared module which is shared between client component error boundary and dynamic component
const BAILOUT_TO_CSR = "BAILOUT_TO_CLIENT_SIDE_RENDERING";
/** An error that should be thrown when we want to bail out to client-side rendering. */ export class BailoutToCSRError extends Error {
    constructor(reason){
        super("Bail out to client-side rendering: " + reason);
        this.reason = reason;
        this.digest = BAILOUT_TO_CSR;
    }
}
/** Checks if a passed argument is an error that is thrown if we want to bail out to client-side rendering. */ export function isBailoutToCSRError(err) {
    if (typeof err !== "object" || err === null || !("digest" in err)) {
        return false;
    }
    return err.digest === BAILOUT_TO_CSR;
}

//# sourceMappingURL=bailout-to-csr.js.map