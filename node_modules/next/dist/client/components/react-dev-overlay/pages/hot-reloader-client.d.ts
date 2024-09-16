/**
 * MIT License
 *
 * Copyright (c) 2013-present, Facebook, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
import type { TurbopackMsgToBrowser } from '../../../../server/dev/hot-reloader-types';
declare global {
    const __webpack_hash__: string;
    interface Window {
        __nextDevClientId: number;
        __NEXT_HMR_LATENCY_CB: any;
    }
}
export default function connect(mode: 'webpack' | 'turbopack'): {
    subscribeToHmrEvent(handler: any): void;
    onUnrecoverableError(): void;
    addTurbopackMessageListener(cb: (msg: TurbopackMsgToBrowser) => void): void;
    sendTurbopackMessage(msg: string): void;
    handleUpdateError(err: unknown): void;
};
export declare function performFullReload(err: any): void;
