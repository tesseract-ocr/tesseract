export const pipe = (...fns)=>(param)=>fns.reduce(async (result, next)=>next(await result), param);

//# sourceMappingURL=utils.js.map