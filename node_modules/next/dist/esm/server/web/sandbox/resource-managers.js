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
export const intervalsManager = new IntervalsManager();
export const timeoutsManager = new TimeoutsManager();

//# sourceMappingURL=resource-managers.js.map