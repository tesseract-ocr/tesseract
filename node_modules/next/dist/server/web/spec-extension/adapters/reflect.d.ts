export declare class ReflectAdapter {
    static get<T extends object>(target: T, prop: string | symbol, receiver: unknown): any;
    static set<T extends object>(target: T, prop: string | symbol, value: any, receiver: any): boolean;
    static has<T extends object>(target: T, prop: string | symbol): boolean;
    static deleteProperty<T extends object>(target: T, prop: string | symbol): boolean;
}
