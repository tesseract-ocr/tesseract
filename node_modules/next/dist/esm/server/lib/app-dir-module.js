import { DEFAULT_SEGMENT_KEY } from "../../shared/lib/segment";
export async function getLayoutOrPageModule(loaderTree) {
    const { layout, page, defaultPage } = loaderTree[2];
    const isLayout = typeof layout !== "undefined";
    const isPage = typeof page !== "undefined";
    const isDefaultPage = typeof defaultPage !== "undefined" && loaderTree[0] === DEFAULT_SEGMENT_KEY;
    let value = undefined;
    let modType = undefined;
    if (isLayout) {
        value = await layout[0]();
        modType = "layout";
    } else if (isPage) {
        value = await page[0]();
        modType = "page";
    } else if (isDefaultPage) {
        value = await defaultPage[0]();
        modType = "page";
    }
    return [
        value,
        modType
    ];
}
export async function getComponentTypeModule(loaderTree, componentType) {
    const { [componentType]: component } = loaderTree[2];
    if (typeof component !== "undefined") {
        return await component[0]();
    }
    return undefined;
}

//# sourceMappingURL=app-dir-module.js.map