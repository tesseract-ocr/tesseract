// Alias index file of next/server for edge runtime for tree-shaking purpose
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    ImageResponse: null,
    NextRequest: null,
    NextResponse: null,
    URLPattern: null,
    userAgent: null,
    userAgentFromString: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    ImageResponse: function() {
        return _imageresponse.ImageResponse;
    },
    NextRequest: function() {
        return _request.NextRequest;
    },
    NextResponse: function() {
        return _response.NextResponse;
    },
    URLPattern: function() {
        return _urlpattern.URLPattern;
    },
    userAgent: function() {
        return _useragent.userAgent;
    },
    userAgentFromString: function() {
        return _useragent.userAgentFromString;
    }
});
const _imageresponse = require("../spec-extension/image-response");
const _request = require("../spec-extension/request");
const _response = require("../spec-extension/response");
const _useragent = require("../spec-extension/user-agent");
const _urlpattern = require("../spec-extension/url-pattern");

//# sourceMappingURL=index.js.map