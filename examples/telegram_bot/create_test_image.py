#!/usr/bin/env python3
"""
Create a test image with text for testing the Tesseract Bot
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_test_image():
    # Create a white image
    width, height = 800, 400
    img = Image.new('RGB', (width, height), color='white')
    draw = ImageDraw.Draw(img)
    
    # Try to use a larger font
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 40)
    except:
        try:
            font = ImageFont.truetype("arial.ttf", 40)
        except:
            font = ImageFont.load_default()
    
    # Add English text
    text_en = "Hello! This is a test image for OCR."
    draw.text((50, 50), text_en, fill='black', font=font)
    
    # Add Arabic text
    text_ar = "مرحبا! هذا اختبار للتعرف الضوئي على الأحرف"
    draw.text((50, 120), text_ar, fill='black', font=font)
    
    # Add some numbers
    text_num = "Test Numbers: 12345 67890"
    draw.text((50, 190), text_num, fill='black', font=font)
    
    # Add instruction
    text_inst = "Send this image to the Tesseract Bot!"
    draw.text((50, 260), text_inst, fill='blue', font=font)
    
    # Save the image
    img.save('test_image.png')
    print("✅ Test image created: test_image.png")
    print("You can now send this image to your Telegram bot for testing!")

if __name__ == '__main__':
    create_test_image()