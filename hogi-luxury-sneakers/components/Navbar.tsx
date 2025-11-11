'use client';

import Link from 'next/link';
import { useState } from 'react';

export default function Navbar() {
  const [isOpen, setIsOpen] = useState(false);

  return (
    <nav className="fixed top-0 left-0 right-0 z-50 bg-black/90 backdrop-blur-md border-b border-gold/20">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-20">
          {/* Logo */}
          <Link href="/" className="flex items-center space-x-2">
            <div className="text-3xl font-bold bg-gradient-to-r from-gold via-yellow-300 to-gold bg-clip-text text-transparent">
              HOGI
            </div>
          </Link>

          {/* Desktop Navigation */}
          <div className="hidden md:flex items-center space-x-8">
            <Link href="/" className="text-white hover:text-gold transition-colors duration-300">
              Home
            </Link>
            <Link href="/shop" className="text-white hover:text-gold transition-colors duration-300">
              Shop
            </Link>
            <Link href="/about" className="text-white hover:text-gold transition-colors duration-300">
              About
            </Link>
            <Link href="/contact" className="text-white hover:text-gold transition-colors duration-300">
              Contact
            </Link>
            <Link 
              href="/cart" 
              className="px-6 py-2 bg-gradient-to-r from-gold to-yellow-600 text-black font-semibold rounded-full hover:shadow-lg hover:shadow-gold/50 transition-all duration-300"
            >
              Cart
            </Link>
          </div>

          {/* Mobile menu button */}
          <button
            onClick={() => setIsOpen(!isOpen)}
            className="md:hidden text-white focus:outline-none"
          >
            <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              {isOpen ? (
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
              ) : (
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 6h16M4 12h16M4 18h16" />
              )}
            </svg>
          </button>
        </div>
      </div>

      {/* Mobile menu */}
      {isOpen && (
        <div className="md:hidden bg-black/95 border-t border-gold/20">
          <div className="px-4 pt-2 pb-4 space-y-2">
            <Link 
              href="/" 
              className="block px-4 py-3 text-white hover:text-gold hover:bg-gold/10 rounded-lg transition-all duration-300"
              onClick={() => setIsOpen(false)}
            >
              Home
            </Link>
            <Link 
              href="/shop" 
              className="block px-4 py-3 text-white hover:text-gold hover:bg-gold/10 rounded-lg transition-all duration-300"
              onClick={() => setIsOpen(false)}
            >
              Shop
            </Link>
            <Link 
              href="/about" 
              className="block px-4 py-3 text-white hover:text-gold hover:bg-gold/10 rounded-lg transition-all duration-300"
              onClick={() => setIsOpen(false)}
            >
              About
            </Link>
            <Link 
              href="/contact" 
              className="block px-4 py-3 text-white hover:text-gold hover:bg-gold/10 rounded-lg transition-all duration-300"
              onClick={() => setIsOpen(false)}
            >
              Contact
            </Link>
            <Link 
              href="/cart" 
              className="block px-4 py-3 bg-gradient-to-r from-gold to-yellow-600 text-black font-semibold rounded-lg text-center"
              onClick={() => setIsOpen(false)}
            >
              Cart
            </Link>
          </div>
        </div>
      )}
    </nav>
  );
}
