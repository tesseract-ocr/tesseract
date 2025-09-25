#!/bin/bash
# Setup script for Tesseract Telegram Bot

echo "🚀 Setting up Tesseract Telegram Bot..."

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is not installed. Please install Python 3.7 or later."
    exit 1
fi

echo "✅ Python 3 found"

# Check if Tesseract is installed
if ! command -v tesseract &> /dev/null; then
    echo "❌ Tesseract is not installed."
    echo "Please install Tesseract OCR:"
    echo "  Ubuntu/Debian: sudo apt install tesseract-ocr"
    echo "  macOS: brew install tesseract"
    echo "  Windows: Download from https://github.com/UB-Mannheim/tesseract/wiki"
    exit 1
fi

echo "✅ Tesseract found: $(tesseract --version | head -n1)"

# Install Python dependencies
echo "📦 Installing Python dependencies..."
pip3 install -r requirements.txt

if [ $? -eq 0 ]; then
    echo "✅ Dependencies installed successfully"
else
    echo "❌ Failed to install dependencies"
    exit 1
fi

# Create config file if it doesn't exist
if [ ! -f "config.py" ]; then
    echo "📝 Creating config.py from template..."
    cp config.py.template config.py
    echo "⚠️  Please edit config.py and add your Telegram bot token"
else
    echo "✅ config.py already exists"
fi

# Check for language packs
echo "🌐 Checking available language packs..."
tesseract --list-langs

echo ""
echo "🎉 Setup complete!"
echo ""
echo "Next steps:"
echo "1. Get a bot token from @BotFather on Telegram"
echo "2. Set your token: export TELEGRAM_BOT_TOKEN='your_token'"
echo "   OR edit config.py with your token"
echo "3. Run the bot: python3 tesseract_bot.py"
echo ""
echo "For more help, see README.md"