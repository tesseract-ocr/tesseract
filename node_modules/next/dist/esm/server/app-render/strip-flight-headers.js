import { FLIGHT_HEADERS } from '../../client/components/app-router-headers';
/**
 * Removes the flight headers from the request.
 *
 * @param req the request to strip the headers from
 */ export function stripFlightHeaders(headers) {
    for (const header of FLIGHT_HEADERS){
        delete headers[header.toLowerCase()];
    }
}

//# sourceMappingURL=strip-flight-headers.js.map