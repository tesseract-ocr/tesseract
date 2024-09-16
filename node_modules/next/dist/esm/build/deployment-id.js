export function getDeploymentIdQueryOrEmptyString() {
    if (process.env.NEXT_DEPLOYMENT_ID) {
        return `?dpl=${process.env.NEXT_DEPLOYMENT_ID}`;
    }
    return "";
}

//# sourceMappingURL=deployment-id.js.map