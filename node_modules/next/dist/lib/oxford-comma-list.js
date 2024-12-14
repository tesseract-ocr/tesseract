"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "getOxfordCommaList", {
    enumerable: true,
    get: function() {
        return getOxfordCommaList;
    }
});
function getOxfordCommaList(items) {
    return items.map((v, index, { length })=>(index > 0 ? index === length - 1 ? length > 2 ? ', and ' : ' and ' : ', ' : '') + v).join('');
}

//# sourceMappingURL=oxford-comma-list.js.map