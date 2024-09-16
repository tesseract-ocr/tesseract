type Handler = (...evts: any[]) => void;
export type MittEmitter<T> = {
    on(type: T, handler: Handler): void;
    off(type: T, handler: Handler): void;
    emit(type: T, ...evts: any[]): void;
};
export default function mitt(): MittEmitter<string>;
export {};
