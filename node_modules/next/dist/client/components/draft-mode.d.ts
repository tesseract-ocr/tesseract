import type { DraftModeProvider } from '../../server/async-storage/draft-mode-provider';
export declare class DraftMode {
    constructor(provider: DraftModeProvider);
    get isEnabled(): boolean;
    enable(): void;
    disable(): void;
}
