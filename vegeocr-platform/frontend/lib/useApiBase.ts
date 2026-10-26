"use client";
import { useEffect, useState } from "react";
import { API } from "./api";

/** The address to show in docs and curl examples: the configured API URL, or this page's own origin when the API is same-origin. */
export function useApiBase() {
  const [base, setBase] = useState(API);
  useEffect(() => { if (!API) setBase(window.location.origin); }, []);
  return base;
}
