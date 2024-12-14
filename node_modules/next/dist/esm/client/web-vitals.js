import { useEffect } from 'react';
import { onLCP, onFID, onCLS, onINP, onFCP, onTTFB } from 'next/dist/compiled/web-vitals';
export function useReportWebVitals(reportWebVitalsFn) {
    useEffect(()=>{
        onCLS(reportWebVitalsFn);
        onFID(reportWebVitalsFn);
        onLCP(reportWebVitalsFn);
        onINP(reportWebVitalsFn);
        onFCP(reportWebVitalsFn);
        onTTFB(reportWebVitalsFn);
    }, [
        reportWebVitalsFn
    ]);
}

//# sourceMappingURL=web-vitals.js.map