const REACT_POSTPONE_TYPE = Symbol.for("react.postpone");
export function isPostpone(error) {
    return typeof error === "object" && error !== null && error.$$typeof === REACT_POSTPONE_TYPE;
}

//# sourceMappingURL=is-postpone.js.map