"use client";
import { useEffect, useState } from "react";
import { blob } from "@/lib/api";

/** <img> for endpoints that need the login token (a plain src can't send it). */
export default function AuthImage({ path, alt, className = "", onClick }: { path: string; alt: string; className?: string; onClick?: () => void }) {
  const [url, setUrl] = useState<string>();
  const [failed, setFailed] = useState(false);
  useEffect(() => {
    let u = "", dead = false;
    setUrl(undefined); setFailed(false);
    blob(path).then(b => { if (!dead) { u = URL.createObjectURL(b); setUrl(u); } }).catch(() => !dead && setFailed(true));
    return () => { dead = true; if (u) URL.revokeObjectURL(u); };
  }, [path]);
  if (failed) return <div className={`grid place-items-center bg-slate-100 text-xs text-slate-400 ${className}`}>image unavailable</div>;
  if (!url) return <div className={`animate-pulse bg-slate-100 ${className}`} aria-label="Loading image" />;
  // eslint-disable-next-line @next/next/no-img-element
  return <img src={url} alt={alt} className={className} onClick={onClick} style={onClick ? { cursor: "zoom-in" } : undefined} />;
}
