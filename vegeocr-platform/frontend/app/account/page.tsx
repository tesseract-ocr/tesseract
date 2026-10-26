"use client";
import AccountPanel from "@/components/AccountPanel";
import Shell from "@/components/Shell";

export default function AccountPage() {
  return <Shell><h1 className="mb-4 text-xl font-bold">Account</h1><AccountPanel scope="user" /></Shell>;
}
