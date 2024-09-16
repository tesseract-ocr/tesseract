// the minimum number of operations required to convert string a to string b.
"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
Object.defineProperty(exports, "detectTypo", {
    enumerable: true,
    get: function() {
        return detectTypo;
    }
});
function minDistance(a, b, threshold) {
    const m = a.length;
    const n = b.length;
    if (m < n) {
        return minDistance(b, a, threshold);
    }
    if (n === 0) {
        return m;
    }
    let previousRow = Array.from({
        length: n + 1
    }, (_, i)=>i);
    for(let i = 0; i < m; i++){
        const s1 = a[i];
        let currentRow = [
            i + 1
        ];
        for(let j = 0; j < n; j++){
            const s2 = b[j];
            const insertions = previousRow[j + 1] + 1;
            const deletions = currentRow[j] + 1;
            const substitutions = previousRow[j] + Number(s1 !== s2);
            currentRow.push(Math.min(insertions, deletions, substitutions));
        }
        previousRow = currentRow;
    }
    return previousRow[previousRow.length - 1];
}
function detectTypo(input, options, threshold = 2) {
    const potentialTypos = options.map((o)=>({
            option: o,
            distance: minDistance(o, input, threshold)
        })).filter(({ distance })=>distance <= threshold && distance > 0).sort((a, b)=>a.distance - b.distance);
    if (potentialTypos.length) {
        return potentialTypos[0].option;
    }
    return null;
}

//# sourceMappingURL=detect-typo.js.map