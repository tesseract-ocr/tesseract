if (!window._nextSetupHydrationWarning) {
    const origConsoleError = window.console.error;
    window.console.error = function() {
        for(var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++){
            args[_key] = arguments[_key];
        }
        const isHydrateError = args.some((arg)=>typeof arg === "string" && arg.match(/(hydration|content does not match|did not match)/i));
        if (isHydrateError) {
            args = [
                ...args,
                "\nSee more info here: https://nextjs.org/docs/messages/react-hydration-error"
            ];
        }
        origConsoleError.apply(window.console, args);
    };
    window._nextSetupHydrationWarning = true;
}
export { };

//# sourceMappingURL=setup-hydration-warning.js.map