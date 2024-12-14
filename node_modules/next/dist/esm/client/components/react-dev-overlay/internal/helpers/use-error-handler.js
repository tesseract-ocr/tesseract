import { useEffect } from 'react';
import { attachHydrationErrorState } from './attach-hydration-error-state';
import { isNextRouterError } from '../../../is-next-router-error';
import { storeHydrationErrorStateFromConsoleArgs } from './hydration-error-info';
import { formatConsoleArgs } from '../../../../lib/console';
import isError from '../../../../../lib/is-error';
import { createUnhandledError } from './console-error';
import { enqueueConsecutiveDedupedError } from './enqueue-client-error';
import { getReactStitchedError } from './stitched-error';
const queueMicroTask = globalThis.queueMicrotask || ((cb)=>Promise.resolve().then(cb));
const errorQueue = [];
const errorHandlers = [];
const rejectionQueue = [];
const rejectionHandlers = [];
export function handleClientError(originError, consoleErrorArgs, capturedFromConsole) {
    if (capturedFromConsole === void 0) capturedFromConsole = false;
    let error;
    if (!originError || !isError(originError)) {
        // If it's not an error, format the args into an error
        const formattedErrorMessage = formatConsoleArgs(consoleErrorArgs);
        error = createUnhandledError(formattedErrorMessage);
    } else {
        error = capturedFromConsole ? createUnhandledError(originError) : originError;
    }
    error = getReactStitchedError(error);
    storeHydrationErrorStateFromConsoleArgs(...consoleErrorArgs);
    attachHydrationErrorState(error);
    enqueueConsecutiveDedupedError(errorQueue, error);
    for (const handler of errorHandlers){
        // Delayed the error being passed to React Dev Overlay,
        // avoid the state being synchronously updated in the component.
        queueMicroTask(()=>{
            handler(error);
        });
    }
}
export function useErrorHandler(handleOnUnhandledError, handleOnUnhandledRejection) {
    useEffect(()=>{
        // Handle queued errors.
        errorQueue.forEach(handleOnUnhandledError);
        rejectionQueue.forEach(handleOnUnhandledRejection);
        // Listen to new errors.
        errorHandlers.push(handleOnUnhandledError);
        rejectionHandlers.push(handleOnUnhandledRejection);
        return ()=>{
            // Remove listeners.
            errorHandlers.splice(errorHandlers.indexOf(handleOnUnhandledError), 1);
            rejectionHandlers.splice(rejectionHandlers.indexOf(handleOnUnhandledRejection), 1);
        };
    }, [
        handleOnUnhandledError,
        handleOnUnhandledRejection
    ]);
}
function onUnhandledError(event) {
    if (isNextRouterError(event.error)) {
        event.preventDefault();
        return false;
    }
    handleClientError(event.error, []);
}
function onUnhandledRejection(ev) {
    const reason = ev == null ? void 0 : ev.reason;
    if (isNextRouterError(reason)) {
        ev.preventDefault();
        return;
    }
    let error = reason;
    if (error && !isError(error)) {
        error = createUnhandledError(error + '');
    }
    rejectionQueue.push(error);
    for (const handler of rejectionHandlers){
        handler(error);
    }
}
export function handleGlobalErrors() {
    if (typeof window !== 'undefined') {
        try {
            // Increase the number of stack frames on the client
            Error.stackTraceLimit = 50;
        } catch (e) {}
        window.addEventListener('error', onUnhandledError);
        window.addEventListener('unhandledrejection', onUnhandledRejection);
    }
}

//# sourceMappingURL=use-error-handler.js.map