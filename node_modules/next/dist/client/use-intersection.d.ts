/// <reference types="react" />
type UseIntersectionObserverInit = Pick<IntersectionObserverInit, 'rootMargin' | 'root'>;
type UseIntersection = {
    disabled?: boolean;
} & UseIntersectionObserverInit & {
    rootRef?: React.RefObject<HTMLElement> | null;
};
export declare function useIntersection<T extends Element>({ rootRef, rootMargin, disabled, }: UseIntersection): [(element: T | null) => void, boolean, () => void];
export {};
