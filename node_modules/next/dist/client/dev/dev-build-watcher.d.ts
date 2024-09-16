export interface ShowHideHandler {
    show: () => void;
    hide: () => void;
}
export default function initializeBuildWatcher(toggleCallback: (handlers: ShowHideHandler) => void, position?: string): void;
