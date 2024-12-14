let warnOnce = (_)=>{};
if (process.env.NODE_ENV !== 'production') {
    const warnings = new Set();
    warnOnce = (msg)=>{
        if (!warnings.has(msg)) {
            console.warn(msg);
        }
        warnings.add(msg);
    };
}
export { warnOnce };

//# sourceMappingURL=warn-once.js.map