import { RouteMatcher } from "./route-matcher";
export class AppPageRouteMatcher extends RouteMatcher {
    get identity() {
        return `${this.definition.pathname}?__nextPage=${this.definition.page}`;
    }
}

//# sourceMappingURL=app-page-route-matcher.js.map