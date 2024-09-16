import { RSC_CONTENT_TYPE_HEADER } from "../../client/components/app-router-headers";
import RenderResult from "../render-result";
/**
 * Flight Response is always set to RSC_CONTENT_TYPE_HEADER to ensure it does not get interpreted as HTML.
 */ export class FlightRenderResult extends RenderResult {
    constructor(response){
        super(response, {
            contentType: RSC_CONTENT_TYPE_HEADER,
            metadata: {}
        });
    }
}

//# sourceMappingURL=flight-render-result.js.map