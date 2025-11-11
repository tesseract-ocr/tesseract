export interface Product {
  id: string;
  name: string;
  price: number;
  description: string;
  category: string;
  colors: string[];
  sizes: number[];
  images: string[];
  featured: boolean;
  new: boolean;
}

export interface CartItem extends Product {
  quantity: number;
  selectedSize: number;
  selectedColor: string;
}
