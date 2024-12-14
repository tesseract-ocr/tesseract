#!/usr/bin/env node
export type NextTelemetryOptions = {
    enable?: boolean;
    disable?: boolean;
};
declare const nextTelemetry: (options: NextTelemetryOptions, arg: string) => void;
export { nextTelemetry };
