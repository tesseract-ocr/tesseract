"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    intervalsManager: null,
    timeoutsManager: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    intervalsManager: function() {
        return intervalsManager;
    },
    timeoutsManager: function() {
        return timeoutsManager;
    }
});
class ResourceManager {
    add(resourceArgs) {
        const resource = this.create(resourceArgs);
        this.resources.push(resource);
        return resource;
    }
    remove(resource) {
        this.resources = this.resources.filter((r)=>r !== resource);
        this.destroy(resource);
    }
    removeAll() {
        this.resources.forEach(this.destroy);
        this.resources = [];
    }
    constructor(){
        this.resources = [];
    }
}
class IntervalsManager extends ResourceManager {
    create(args) {
        // TODO: use the edge runtime provided `setInterval` instead
        return setInterval(...args)[Symbol.toPrimitive]();
    }
    destroy(interval) {
        clearInterval(interval);
    }
}
class TimeoutsManager extends ResourceManager {
    create(args) {
        // TODO: use the edge runtime provided `setTimeout` instead
        return setTimeout(...args)[Symbol.toPrimitive]();
    }
    destroy(timeout) {
        clearTimeout(timeout);
    }
}
const intervalsManager = new IntervalsManager();
const timeoutsManager = new TimeoutsManager();

//# sourceMappingURL=resource-managers.js.map