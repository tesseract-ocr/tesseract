/** @type {import('tailwindcss').Config} */
module.exports = {
  content: ["./app/**/*.{ts,tsx}", "./components/**/*.{ts,tsx}"],
  theme: { extend: { colors: { brand: { 50: "#e8f5e9", 100: "#c8e6c9", 500: "#43a047", 600: "#2e7d32", 700: "#1f5c23" } } } },
  plugins: [],
};
