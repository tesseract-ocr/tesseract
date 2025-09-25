# Tesseract Telegram Bot

A Telegram bot that uses Tesseract OCR to extract text from images. Users can send images to the bot and receive the extracted text back, with support for multiple languages.

## Features

- 📸 **Image OCR**: Extract text from images using Tesseract
- 🌐 **Multi-language Support**: Supports Arabic, English, German, French, Spanish, Russian, Chinese, and Japanese
- 🔧 **Easy Configuration**: Simple setup with environment variables or config file
- 📊 **Confidence Scoring**: Shows OCR confidence when available
- 🛡️ **Error Handling**: Robust error handling and user feedback
- 📱 **User-friendly Interface**: Inline keyboards for language selection

## Supported Languages

- 🇺🇸 English (eng)
- 🇸🇦 Arabic (ara)
- 🇩🇪 German (deu)
- 🇫🇷 French (fra)
- 🇪🇸 Spanish (spa)
- 🇷🇺 Russian (rus)
- 🇨🇳 Chinese Simplified (chi_sim)
- 🇯🇵 Japanese (jpn)

## Prerequisites

1. **Python 3.7+**
2. **Tesseract OCR** installed on your system
3. **Language data files** for the languages you want to support
4. **Telegram Bot Token** from @BotFather

## Installation

### 1. Install Tesseract OCR

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install tesseract-ocr
# Install language packs (example for Arabic and German)
sudo apt install tesseract-ocr-ara tesseract-ocr-deu
```

#### macOS:
```bash
brew install tesseract
# Install language packs
brew install tesseract-lang
```

#### Windows:
Download from: https://github.com/UB-Mannheim/tesseract/wiki

### 2. Install Python Dependencies

```bash
cd examples/telegram_bot
pip install -r requirements.txt
```

### 3. Create Your Bot

1. Message [@BotFather](https://t.me/botfather) on Telegram
2. Send `/newbot` and follow the instructions
3. Choose a name and username for your bot
4. Save the bot token provided by BotFather

### 4. Configure the Bot

#### Option A: Environment Variable (Recommended)
```bash
export TELEGRAM_BOT_TOKEN="your_bot_token_here"
```

#### Option B: Configuration File
```bash
cp config.py.template config.py
# Edit config.py and add your bot token
```

## Usage

### Starting the Bot

```bash
python tesseract_bot.py
```

### Using the Bot

1. **Start**: Send `/start` to begin
2. **Send Image**: Send any image (photo or document)
3. **Change Language**: Use `/language` to select OCR language
4. **Get Help**: Use `/help` for detailed instructions
5. **Check Status**: Use `/status` to verify bot status

### Commands

- `/start` - Welcome message and instructions
- `/help` - Detailed help and tips
- `/language` - Change OCR language
- `/status` - Check bot and Tesseract status

## Configuration Options

Edit `config.py` to customize:

```python
# Bot token
TELEGRAM_BOT_TOKEN = 'your_token_here'

# Tesseract path (if not in PATH)
TESSERACT_PATH = '/usr/bin/tesseract'

# Additional Tesseract options
TESSERACT_CONFIG = '--oem 3 --psm 6'

# Supported languages
SUPPORTED_LANGUAGES = {
    'eng': 'English',
    'ara': 'Arabic',
    # Add more languages...
}

# Default language
DEFAULT_LANGUAGE = 'eng'

# Maximum file size (bytes)
MAX_FILE_SIZE = 20 * 1024 * 1024  # 20MB

# Debug mode
DEBUG = False
```

## Tips for Better OCR Results

1. **Image Quality**: Use high-resolution, clear images
2. **Contrast**: Ensure good contrast between text and background
3. **Orientation**: Keep text horizontal when possible
4. **Language**: Select the correct language for your text
5. **Format**: PNG and TIFF usually give better results than JPEG

## Troubleshooting

### Common Issues

1. **"Tesseract not found"**
   - Install Tesseract OCR on your system
   - Add Tesseract to your PATH or set TESSERACT_PATH in config

2. **"Language not supported"**
   - Install the required language pack
   - Ubuntu: `sudo apt install tesseract-ocr-ara` (for Arabic)

3. **"No text found"**
   - Try a clearer image
   - Check if the correct language is selected
   - Ensure good contrast in the image

4. **"Bot doesn't respond"**
   - Check your bot token
   - Verify internet connection
   - Check bot logs for errors

### Debugging

Enable debug mode in `config.py`:
```python
DEBUG = True
```

Run with verbose logging:
```bash
python tesseract_bot.py
```

## Security Notes

⚠️ **Important Security Guidelines:**

1. **Never commit your bot token to version control**
2. **Use environment variables for sensitive data**
3. **Keep your bot token private**
4. **Regularly rotate your bot token if compromised**

## Docker Support

Create a `Dockerfile`:

```dockerfile
FROM python:3.9-slim

# Install Tesseract
RUN apt-get update && apt-get install -y \
    tesseract-ocr \
    tesseract-ocr-eng \
    tesseract-ocr-ara \
    && rm -rf /var/lib/apt/lists/*

# Copy application
WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . .

# Run bot
CMD ["python", "tesseract_bot.py"]
```

Build and run:
```bash
docker build -t tesseract-bot .
docker run -e TELEGRAM_BOT_TOKEN="your_token" tesseract-bot
```

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](../../LICENSE) file for details.

## Support

For issues and questions:

1. Check this README first
2. Look at the [Tesseract documentation](https://tesseract-ocr.github.io/tessdoc/)
3. Open an issue on GitHub
4. Check Telegram Bot API documentation

## Changelog

### v1.0.0
- Initial release
- Multi-language OCR support
- Telegram bot integration
- Configuration system
- Error handling and user feedback