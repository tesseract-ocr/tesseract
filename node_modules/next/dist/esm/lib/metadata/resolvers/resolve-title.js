function resolveTitleTemplate(template, title) {
    return template ? template.replace(/%s/g, title) : title;
}
export function resolveTitle(title, stashedTemplate) {
    let resolved;
    const template = typeof title !== "string" && title && "template" in title ? title.template : null;
    if (typeof title === "string") {
        resolved = resolveTitleTemplate(stashedTemplate, title);
    } else if (title) {
        if ("default" in title) {
            resolved = resolveTitleTemplate(stashedTemplate, title.default);
        }
        if ("absolute" in title && title.absolute) {
            resolved = title.absolute;
        }
    }
    if (title && typeof title !== "string") {
        return {
            template,
            absolute: resolved || ""
        };
    } else {
        return {
            absolute: resolved || title || "",
            template
        };
    }
}

//# sourceMappingURL=resolve-title.js.map