export type ComponentStackFrame = {
    canOpenInEditor: boolean;
    component: string;
    file?: string;
    lineNumber?: number;
    column?: number;
};
export declare function parseComponentStack(componentStack: string): ComponentStackFrame[];
