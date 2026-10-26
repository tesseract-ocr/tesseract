import "./globals.css";
import type { ReactNode } from "react";

export const metadata = { title: "VegeOCR", description: "OCR for images and PDFs" };

export default function RootLayout({ children }: { children: ReactNode }) {
  return <html lang="en"><body>{children}</body></html>;
}
