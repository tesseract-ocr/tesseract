#!/usr/bin/env node
type NextTelemetryOptions = {
    enable?: boolean;
    disable?: boolean;
};
declare const nextTelemetry: (options: NextTelemetryOptions, arg: string) => void;
export { nextTelemetry };
