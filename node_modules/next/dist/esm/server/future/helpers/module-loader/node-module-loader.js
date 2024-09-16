/**
 * Loads a module using `await require(id)`.
 */ export class NodeModuleLoader {
    async load(id) {
        if (process.env.NEXT_RUNTIME !== "edge") {
            // Need to `await` to cover the case that route is marked ESM modules by ESM escalation.
            return await (process.env.NEXT_MINIMAL ? __non_webpack_require__(id) : require(id));
        }
        throw new Error("NodeModuleLoader is not supported in edge runtime.");
    }
}

//# sourceMappingURL=node-module-loader.js.map