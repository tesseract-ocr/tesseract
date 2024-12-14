import { Worker as JestWorker } from 'next/dist/compiled/jest-worker';
import { Transform } from 'stream';
const RESTARTED = Symbol('restarted');
const cleanupWorkers = (worker)=>{
    var _worker__workerPool;
    for (const curWorker of ((_worker__workerPool = worker._workerPool) == null ? void 0 : _worker__workerPool._workers) || []){
        var _curWorker__child;
        (_curWorker__child = curWorker._child) == null ? void 0 : _curWorker__child.kill('SIGINT');
    }
};
export class Worker {
    constructor(workerPath, options){
        let { timeout, onRestart, logger = console, ...farmOptions } = options;
        let restartPromise;
        let resolveRestartPromise;
        let activeTasks = 0;
        this._worker = undefined;
        // ensure we end workers if they weren't before exit
        process.on('exit', ()=>{
            this.close();
        });
        const createWorker = ()=>{
            var _farmOptions_forkOptions;
            this._worker = new JestWorker(workerPath, {
                ...farmOptions,
                forkOptions: {
                    ...farmOptions.forkOptions,
                    env: {
                        ...((_farmOptions_forkOptions = farmOptions.forkOptions) == null ? void 0 : _farmOptions_forkOptions.env) || {},
                        ...process.env
                    }
                },
                maxRetries: 0
            });
            restartPromise = new Promise((resolve)=>resolveRestartPromise = resolve);
            /**
       * Jest Worker has two worker types, ChildProcessWorker (uses child_process) and NodeThreadWorker (uses worker_threads)
       * Next.js uses ChildProcessWorker by default, but it can be switched to NodeThreadWorker with an experimental flag
       *
       * We only want to handle ChildProcessWorker's orphan process issue, so we access the private property "_child":
       * https://github.com/facebook/jest/blob/b38d7d345a81d97d1dc3b68b8458b1837fbf19be/packages/jest-worker/src/workers/ChildProcessWorker.ts
       *
       * But this property is not available in NodeThreadWorker, so we need to check if we are using ChildProcessWorker
       */ if (!farmOptions.enableWorkerThreads) {
                var _this__worker__workerPool;
                for (const worker of ((_this__worker__workerPool = this._worker._workerPool) == null ? void 0 : _this__worker__workerPool._workers) || []){
                    var _worker__child, // if a child process emits a particular message, we track that as activity
                    // so the parent process can keep track of progress
                    _worker__child1;
                    (_worker__child = worker._child) == null ? void 0 : _worker__child.on('exit', (code, signal)=>{
                        if ((code || signal && signal !== 'SIGINT') && this._worker) {
                            logger.error(`Static worker exited with code: ${code} and signal: ${signal}`);
                            // if a child process doesn't exit gracefully, we want to bubble up the exit code to the parent process
                            process.exit(code ?? 1);
                        }
                    });
                    (_worker__child1 = worker._child) == null ? void 0 : _worker__child1.on('message', ([, data])=>{
                        if (data && typeof data === 'object' && 'type' in data && data.type === 'activity') {
                            onActivity();
                        }
                    });
                }
            }
            let aborted = false;
            const onActivityAbort = ()=>{
                if (!aborted) {
                    options.onActivityAbort == null ? void 0 : options.onActivityAbort.call(options);
                    aborted = true;
                }
            };
            // Listen to the worker's stdout and stderr, if there's any thing logged, abort the activity first
            const abortActivityStreamOnLog = new Transform({
                transform (_chunk, _encoding, callback) {
                    onActivityAbort();
                    callback();
                }
            });
            // Stop the activity if there's any output from the worker
            this._worker.getStdout().pipe(abortActivityStreamOnLog);
            this._worker.getStderr().pipe(abortActivityStreamOnLog);
            // Pipe the worker's stdout and stderr to the parent process
            this._worker.getStdout().pipe(process.stdout);
            this._worker.getStderr().pipe(process.stderr);
        };
        createWorker();
        const onHanging = ()=>{
            const worker = this._worker;
            if (!worker) return;
            const resolve = resolveRestartPromise;
            createWorker();
            logger.warn(`Sending SIGTERM signal to static worker due to timeout${timeout ? ` of ${timeout / 1000} seconds` : ''}. Subsequent errors may be a result of the worker exiting.`);
            worker.end().then(()=>{
                resolve(RESTARTED);
            });
        };
        let hangingTimer = false;
        const onActivity = ()=>{
            if (hangingTimer) clearTimeout(hangingTimer);
            if (options.onActivity) options.onActivity();
            hangingTimer = activeTasks > 0 && setTimeout(onHanging, timeout);
        };
        for (const method of farmOptions.exposedMethods){
            if (method.startsWith('_')) continue;
            this[method] = timeout ? async (...args)=>{
                activeTasks++;
                try {
                    let attempts = 0;
                    for(;;){
                        onActivity();
                        const result = await Promise.race([
                            this._worker[method](...args),
                            restartPromise
                        ]);
                        if (result !== RESTARTED) return result;
                        if (onRestart) onRestart(method, args, ++attempts);
                    }
                } finally{
                    activeTasks--;
                    onActivity();
                }
            } : this._worker[method].bind(this._worker);
        }
    }
    end() {
        const worker = this._worker;
        if (!worker) {
            throw new Error('Farm is ended, no more calls can be done to it');
        }
        cleanupWorkers(worker);
        this._worker = undefined;
        return worker.end();
    }
    /**
   * Quietly end the worker if it exists
   */ close() {
        if (this._worker) {
            cleanupWorkers(this._worker);
            this._worker.end();
        }
    }
}

//# sourceMappingURL=worker.js.map