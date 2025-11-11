'use client';

import { useState, useEffect } from 'react';
import { useRouter } from 'next/navigation';
import { CartItem } from '@/types/product';

export default function CheckoutPage() {
  const router = useRouter();
  const [cartItems, setCartItems] = useState<CartItem[]>([]);
  const [formData, setFormData] = useState({
    firstName: '',
    lastName: '',
    email: '',
    phone: '',
    address: '',
    city: '',
    state: '',
    zipCode: '',
    country: '',
    cardNumber: '',
    expiryDate: '',
    cvv: '',
  });

  useEffect(() => {
    const cart = JSON.parse(localStorage.getItem('cart') || '[]');
    if (cart.length === 0) {
      router.push('/cart');
    }
    setCartItems(cart);
  }, [router]);

  const subtotal = cartItems.reduce((sum, item) => sum + item.price * item.quantity, 0);
  const tax = subtotal * 0.08;
  const total = subtotal + tax;

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    alert('Order placed successfully! This is a demo checkout.');
    localStorage.removeItem('cart');
    router.push('/');
  };

  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setFormData({ ...formData, [e.target.name]: e.target.value });
  };

  return (
    <div className="pt-20 min-h-screen bg-gradient-to-b from-black via-gray-900 to-black">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-12">
        <h1 className="text-4xl md:text-5xl font-bold text-white mb-12">Checkout</h1>

        <form onSubmit={handleSubmit} className="grid grid-cols-1 lg:grid-cols-3 gap-8">
          {/* Checkout Form */}
          <div className="lg:col-span-2 space-y-8">
            {/* Contact Information */}
            <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-6">
              <h2 className="text-2xl font-bold text-white mb-6">Contact Information</h2>
              <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                <input
                  type="text"
                  name="firstName"
                  placeholder="First Name"
                  required
                  value={formData.firstName}
                  onChange={handleChange}
                  className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                />
                <input
                  type="text"
                  name="lastName"
                  placeholder="Last Name"
                  required
                  value={formData.lastName}
                  onChange={handleChange}
                  className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                />
                <input
                  type="email"
                  name="email"
                  placeholder="Email"
                  required
                  value={formData.email}
                  onChange={handleChange}
                  className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                />
                <input
                  type="tel"
                  name="phone"
                  placeholder="Phone"
                  required
                  value={formData.phone}
                  onChange={handleChange}
                  className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                />
              </div>
            </div>

            {/* Shipping Address */}
            <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-6">
              <h2 className="text-2xl font-bold text-white mb-6">Shipping Address</h2>
              <div className="space-y-4">
                <input
                  type="text"
                  name="address"
                  placeholder="Street Address"
                  required
                  value={formData.address}
                  onChange={handleChange}
                  className="w-full px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                />
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                  <input
                    type="text"
                    name="city"
                    placeholder="City"
                    required
                    value={formData.city}
                    onChange={handleChange}
                    className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                  />
                  <input
                    type="text"
                    name="state"
                    placeholder="State/Province"
                    required
                    value={formData.state}
                    onChange={handleChange}
                    className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                  />
                  <input
                    type="text"
                    name="zipCode"
                    placeholder="ZIP/Postal Code"
                    required
                    value={formData.zipCode}
                    onChange={handleChange}
                    className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                  />
                  <input
                    type="text"
                    name="country"
                    placeholder="Country"
                    required
                    value={formData.country}
                    onChange={handleChange}
                    className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                  />
                </div>
              </div>
            </div>

            {/* Payment Information */}
            <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-6">
              <h2 className="text-2xl font-bold text-white mb-6">Payment Information</h2>
              <div className="space-y-4">
                <input
                  type="text"
                  name="cardNumber"
                  placeholder="Card Number"
                  required
                  value={formData.cardNumber}
                  onChange={handleChange}
                  className="w-full px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                />
                <div className="grid grid-cols-2 gap-4">
                  <input
                    type="text"
                    name="expiryDate"
                    placeholder="MM/YY"
                    required
                    value={formData.expiryDate}
                    onChange={handleChange}
                    className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                  />
                  <input
                    type="text"
                    name="cvv"
                    placeholder="CVV"
                    required
                    value={formData.cvv}
                    onChange={handleChange}
                    className="px-4 py-3 bg-gray-800 text-white rounded-lg border border-gray-700 focus:border-gold focus:outline-none"
                  />
                </div>
              </div>
            </div>
          </div>

          {/* Order Summary */}
          <div className="lg:col-span-1">
            <div className="bg-gradient-to-br from-gray-900 to-black rounded-2xl border border-gold/20 p-6 sticky top-24">
              <h2 className="text-2xl font-bold text-white mb-6">Order Summary</h2>

              <div className="space-y-4 mb-6 max-h-64 overflow-y-auto">
                {cartItems.map((item, index) => (
                  <div key={index} className="flex justify-between text-sm">
                    <div className="flex-1">
                      <div className="text-white font-semibold">{item.name}</div>
                      <div className="text-gray-400 text-xs">
                        {item.selectedColor} • Size {item.selectedSize} • Qty {item.quantity}
                      </div>
                    </div>
                    <div className="text-gold font-semibold">
                      ${(item.price * item.quantity).toLocaleString()}
                    </div>
                  </div>
                ))}
              </div>

              <div className="space-y-3 border-t border-gold/20 pt-4">
                <div className="flex justify-between text-gray-300">
                  <span>Subtotal</span>
                  <span>${subtotal.toLocaleString()}</span>
                </div>
                <div className="flex justify-between text-gray-300">
                  <span>Shipping</span>
                  <span className="text-gold">FREE</span>
                </div>
                <div className="flex justify-between text-gray-300">
                  <span>Tax</span>
                  <span>${tax.toFixed(2)}</span>
                </div>
                <div className="border-t border-gold/20 pt-3 flex justify-between text-xl font-bold">
                  <span className="text-white">Total</span>
                  <span className="text-gold">${total.toFixed(2)}</span>
                </div>
              </div>

              <button
                type="submit"
                className="w-full mt-6 py-4 bg-gradient-to-r from-gold to-yellow-600 text-black font-bold rounded-full hover:shadow-2xl hover:shadow-gold/50 transition-all duration-300"
              >
                Place Order
              </button>

              <div className="mt-6 text-xs text-gray-400 text-center">
                By placing your order, you agree to our terms and conditions
              </div>
            </div>
          </div>
        </form>
      </div>
    </div>
  );
}
