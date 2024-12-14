export function needsExperimentalReact(config) {
    const { ppr, taint, reactOwnerStack } = config.experimental || {};
    return Boolean(ppr || taint || reactOwnerStack);
}

//# sourceMappingURL=needs-experimental-react.js.map