"use client";
import { useRouter } from "next/navigation";
import { useEffect } from "react";
import { getSession } from "@/lib/api";

// The site opens on the login page; people who are already signed in go straight to the app.
export default function Home() {
  const router = useRouter();
  useEffect(() => { router.replace(getSession("user") ? "/dashboard" : "/login"); }, [router]);
  return null;
}
