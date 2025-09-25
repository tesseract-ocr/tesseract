#!/usr/bin/env python3
"""
Telegram Bot for Tesseract OCR
A simple bot that processes images sent by users and returns the extracted text using Tesseract OCR.

Requirements:
- python-telegram-bot
- pytesseract
- Pillow
- requests

Usage:
1. Install dependencies: pip install python-telegram-bot pytesseract Pillow requests
2. Set your bot token in config.py or as environment variable TELEGRAM_BOT_TOKEN
3. Run: python tesseract_bot.py

Author: Created for Tesseract OCR integration
License: Apache License 2.0
"""

import os
import logging
import tempfile
from io import BytesIO
from typing import Optional

import requests
from PIL import Image
from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup, BotCommand
from telegram.ext import (
    Application, 
    CommandHandler, 
    MessageHandler, 
    CallbackQueryHandler,
    filters, 
    ContextTypes
)

try:
    import pytesseract
except ImportError:
    print("pytesseract is not installed. Please install it with: pip install pytesseract")
    exit(1)

# Configure logging
logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)
logger = logging.getLogger(__name__)

class TesseractBot:
    """Telegram bot for OCR using Tesseract"""
    
    def __init__(self, token: str):
        self.token = token
        self.supported_languages = {
            'eng': 'English',
            'ara': 'Arabic', 
            'deu': 'German',
            'fra': 'French',
            'spa': 'Spanish',
            'rus': 'Russian',
            'chi_sim': 'Chinese (Simplified)',
            'jpn': 'Japanese'
        }
        self.default_language = 'eng'
    
    def get_language_keyboard(self):
        """Create inline keyboard for language selection"""
        keyboard = []
        row = []
        for code, name in self.supported_languages.items():
            button = InlineKeyboardButton(f"{name}", callback_data=f"lang_{code}")
            row.append(button)
            if len(row) == 2:
                keyboard.append(row)
                row = []
        if row:
            keyboard.append(row)
        
        keyboard.append([InlineKeyboardButton("🔙 Use Default (English)", callback_data="lang_eng")])
        return InlineKeyboardMarkup(keyboard)
    
    async def start_command(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Handle /start command"""
        welcome_message = """
🤖 *Welcome to Tesseract OCR Bot!*

I can extract text from images using Tesseract OCR engine.

*How to use:*
• Send me an image and I'll extract the text from it
• Use /language to change OCR language
• Use /help for more information

*Supported formats:* PNG, JPEG, GIF, BMP, TIFF

Just send me an image to get started! 📸
        """
        await update.message.reply_text(
            welcome_message, 
            parse_mode='Markdown'
        )
    
    async def help_command(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Handle /help command"""
        help_message = """
📖 *Help - Tesseract OCR Bot*

*Commands:*
• `/start` - Show welcome message
• `/help` - Show this help message
• `/language` - Change OCR language
• `/status` - Check bot status

*How it works:*
1. Send me an image (photo, document, or file)
2. I'll process it using Tesseract OCR
3. You'll get the extracted text back

*Supported Languages:*
🇺🇸 English, 🇸🇦 Arabic, 🇩🇪 German, 🇫🇷 French, 
🇪🇸 Spanish, 🇷🇺 Russian, 🇨🇳 Chinese, 🇯🇵 Japanese

*Tips for better results:*
• Use high-quality images
• Ensure good contrast between text and background
• Keep text horizontal if possible
• Avoid blurry or distorted images

*Current language:* {current_lang}
        """.format(current_lang=self.supported_languages.get(
            context.user_data.get('language', self.default_language), 
            'English'
        ))
        
        await update.message.reply_text(help_message, parse_mode='Markdown')
    
    async def language_command(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Handle /language command"""
        current_lang = context.user_data.get('language', self.default_language)
        current_lang_name = self.supported_languages.get(current_lang, 'English')
        
        message = f"🌐 *Language Selection*\n\nCurrent language: *{current_lang_name}*\n\nChoose a new language:"
        
        await update.message.reply_text(
            message,
            parse_mode='Markdown',
            reply_markup=self.get_language_keyboard()
        )
    
    async def status_command(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Handle /status command"""
        current_lang = context.user_data.get('language', self.default_language)
        current_lang_name = self.supported_languages.get(current_lang, 'English')
        
        try:
            tesseract_version = pytesseract.get_tesseract_version()
        except Exception:
            tesseract_version = "Unable to detect"
        
        status_message = f"""
✅ *Bot Status*

🤖 Bot: Online and ready
🔧 Tesseract Version: {tesseract_version}
🌐 Current Language: {current_lang_name} ({current_lang})
📊 Supported Languages: {len(self.supported_languages)}

Everything looks good! Send me an image to test OCR.
        """
        
        await update.message.reply_text(status_message, parse_mode='Markdown')
    
    async def language_callback(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Handle language selection callback"""
        query = update.callback_query
        await query.answer()
        
        language_code = query.data.replace('lang_', '')
        if language_code in self.supported_languages:
            context.user_data['language'] = language_code
            language_name = self.supported_languages[language_code]
            
            await query.edit_message_text(
                f"✅ Language changed to: *{language_name}*\n\nSend me an image to test OCR with the new language!",
                parse_mode='Markdown'
            )
        else:
            await query.edit_message_text("❌ Invalid language selection.")
    
    async def process_image(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Process received images"""
        try:
            # Show processing message
            processing_msg = await update.message.reply_text("🔄 Processing image... Please wait.")
            
            # Get the image file
            if update.message.photo:
                # Get the largest photo
                photo = update.message.photo[-1]
                file = await context.bot.get_file(photo.file_id)
            elif update.message.document:
                # Handle document
                file = await context.bot.get_file(update.message.document.file_id)
            else:
                await processing_msg.edit_text("❌ Please send an image file.")
                return
            
            # Download the image
            file_url = file.file_path
            response = requests.get(file_url)
            
            if response.status_code != 200:
                await processing_msg.edit_text("❌ Failed to download image.")
                return
            
            # Open image with PIL
            image = Image.open(BytesIO(response.content))
            
            # Get user's preferred language
            language = context.user_data.get('language', self.default_language)
            
            # Perform OCR
            try:
                text = pytesseract.image_to_string(image, lang=language)
            except pytesseract.TesseractNotFoundError:
                await processing_msg.edit_text(
                    "❌ Tesseract is not properly installed. Please check the installation."
                )
                return
            except pytesseract.TesseractError as e:
                await processing_msg.edit_text(f"❌ OCR Error: {str(e)}")
                return
            
            # Process the extracted text
            text = text.strip()
            
            if not text:
                await processing_msg.edit_text(
                    "📝 No text found in the image.\n\n"
                    "💡 *Tips:*\n"
                    "• Ensure good image quality\n"
                    "• Check if the language setting is correct\n"
                    "• Try with a clearer image"
                )
                return
            
            # Format response
            language_name = self.supported_languages.get(language, 'English')
            response_text = f"📝 *Extracted Text* ({language_name}):\n\n"
            
            # Limit text length for Telegram message
            if len(text) > 3500:
                response_text += f"{text[:3500]}...\n\n⚠️ *Text truncated* (too long for single message)"
            else:
                response_text += f"{text}"
            
            # Add confidence info if available
            try:
                confidence_data = pytesseract.image_to_data(image, lang=language, output_type=pytesseract.Output.DICT)
                confidences = [int(conf) for conf in confidence_data['conf'] if int(conf) > 0]
                if confidences:
                    avg_confidence = sum(confidences) / len(confidences)
                    response_text += f"\n\n📊 *Confidence:* {avg_confidence:.1f}%"
            except Exception:
                pass  # Skip confidence calculation if it fails
            
            await processing_msg.edit_text(response_text, parse_mode='Markdown')
            
        except Exception as e:
            logger.error(f"Error processing image: {e}")
            try:
                await processing_msg.edit_text(
                    f"❌ An error occurred while processing the image:\n`{str(e)}`"
                )
            except:
                await update.message.reply_text(
                    f"❌ An error occurred while processing the image:\n`{str(e)}`"
                )
    
    async def handle_unsupported(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Handle unsupported message types"""
        await update.message.reply_text(
            "📸 Please send me an image (photo or document) to extract text from it.\n\n"
            "Use /help for more information."
        )
    
    def run(self):
        """Start the bot"""
        logger.info("Starting Tesseract OCR Bot...")
        
        # Create application
        application = Application.builder().token(self.token).build()
        
        # Add command handlers
        application.add_handler(CommandHandler("start", self.start_command))
        application.add_handler(CommandHandler("help", self.help_command))
        application.add_handler(CommandHandler("language", self.language_command))
        application.add_handler(CommandHandler("status", self.status_command))
        
        # Add callback handler for language selection
        application.add_handler(CallbackQueryHandler(self.language_callback, pattern="^lang_"))
        
        # Add message handlers
        application.add_handler(MessageHandler(
            filters.PHOTO | filters.Document.IMAGE, 
            self.process_image
        ))
        application.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, self.handle_unsupported))
        
        # Set bot commands for menu
        async def set_commands():
            commands = [
                BotCommand("start", "Start the bot"),
                BotCommand("help", "Show help message"),
                BotCommand("language", "Change OCR language"), 
                BotCommand("status", "Check bot status")
            ]
            await application.bot.set_my_commands(commands)
        
        # Run the bot
        application.run_polling(drop_pending_updates=True)


def main():
    """Main function"""
    # Try to get token from environment variable
    token = os.getenv('TELEGRAM_BOT_TOKEN')
    
    # If not found, try to import from config file
    if not token:
        try:
            from config import TELEGRAM_BOT_TOKEN
            token = TELEGRAM_BOT_TOKEN
        except ImportError:
            print("❌ Bot token not found!")
            print("\nPlease set your bot token in one of these ways:")
            print("1. Set environment variable: export TELEGRAM_BOT_TOKEN='your_token'")
            print("2. Create config.py file with: TELEGRAM_BOT_TOKEN = 'your_token'")
            print("\nTo get a bot token:")
            print("1. Message @BotFather on Telegram")
            print("2. Create a new bot with /newbot")
            print("3. Copy the token provided")
            return
    
    if not token or token == 'YOUR_BOT_TOKEN_HERE':
        print("❌ Please set a valid bot token!")
        return
    
    # Create and run bot
    bot = TesseractBot(token)
    
    try:
        bot.run()
    except KeyboardInterrupt:
        logger.info("Bot stopped by user")
    except Exception as e:
        logger.error(f"Bot error: {e}")


if __name__ == '__main__':
    main()