import Link from 'next/link';

export default function Footer() {
  return (
    <footer className="bg-black border-t border-gold/20 text-white">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-12">
        <div className="grid grid-cols-1 md:grid-cols-4 gap-8">
          {/* Brand */}
          <div className="space-y-4">
            <h3 className="text-2xl font-bold bg-gradient-to-r from-gold via-yellow-300 to-gold bg-clip-text text-transparent">
              HOGI
            </h3>
            <p className="text-gray-400 text-sm">
              Luxury sneakers crafted for those who demand excellence. Each pair is a masterpiece of design and craftsmanship.
            </p>
          </div>

          {/* Quick Links */}
          <div>
            <h4 className="text-gold font-semibold mb-4">Quick Links</h4>
            <ul className="space-y-2">
              <li>
                <Link href="/shop" className="text-gray-400 hover:text-gold transition-colors duration-300">
                  Shop All
                </Link>
              </li>
              <li>
                <Link href="/about" className="text-gray-400 hover:text-gold transition-colors duration-300">
                  About Us
                </Link>
              </li>
              <li>
                <Link href="/contact" className="text-gray-400 hover:text-gold transition-colors duration-300">
                  Contact
                </Link>
              </li>
            </ul>
          </div>

          {/* Customer Service */}
          <div>
            <h4 className="text-gold font-semibold mb-4">Customer Service</h4>
            <ul className="space-y-2">
              <li className="text-gray-400">Shipping & Returns</li>
              <li className="text-gray-400">Size Guide</li>
              <li className="text-gray-400">Care Instructions</li>
              <li className="text-gray-400">FAQ</li>
            </ul>
          </div>

          {/* Contact */}
          <div>
            <h4 className="text-gold font-semibold mb-4">Contact Us</h4>
            <ul className="space-y-2 text-gray-400 text-sm">
              <li>Email: info@hogiluxury.com</li>
              <li>Phone: +1 (555) 123-4567</li>
              <li>Hours: Mon-Fri 9AM-6PM EST</li>
            </ul>
          </div>
        </div>

        <div className="mt-12 pt-8 border-t border-gold/20 text-center text-gray-400 text-sm">
          <p>&copy; {new Date().getFullYear()} Hogi Luxury Sneakers. All rights reserved.</p>
        </div>
      </div>
    </footer>
  );
}
