import { RouteModule } from "../route-module";
import { renderToHTMLImpl, renderToHTML } from "../../../render";
import * as vendoredContexts from "./vendored/contexts/entrypoints";
export class PagesRouteModule extends RouteModule {
    constructor(options){
        super(options);
        this.components = options.components;
    }
    render(req, res, context) {
        return renderToHTMLImpl(req, res, context.page, context.query, context.renderOpts, {
            App: this.components.App,
            Document: this.components.Document
        });
    }
}
const vendored = {
    contexts: vendoredContexts
};
// needed for the static build
export { renderToHTML, vendored };
export default PagesRouteModule;

//# sourceMappingURL=module.js.map