#!/usr/bin/env python3
"""
Configuration validator for Tesseract Telegram Bot
Checks if the configuration is properly set up before running the bot.
"""

import os
import sys

def validate_config():
    """Validate bot configuration"""
    print("🔍 Validating Tesseract Telegram Bot configuration...\n")
    
    issues = []
    warnings = []
    
    # Check for bot token
    token_env = os.getenv('TELEGRAM_BOT_TOKEN')
    token_config = None
    
    try:
        from config import TELEGRAM_BOT_TOKEN
        token_config = TELEGRAM_BOT_TOKEN
    except ImportError:
        pass
    
    if not token_env and not token_config:
        issues.append("❌ No bot token found. Set TELEGRAM_BOT_TOKEN environment variable or create config.py")
    elif token_env and token_env == 'your_bot_token_here':
        issues.append("❌ Bot token is still set to placeholder value")
    elif token_config and token_config == 'YOUR_BOT_TOKEN_HERE':
        issues.append("❌ Bot token in config.py is still set to placeholder value")
    else:
        print("✅ Bot token is configured")
    
    # Check for Tesseract installation
    tesseract_available = True
    try:
        import subprocess
        result = subprocess.run(['tesseract', '--version'], 
                              capture_output=True, text=True, timeout=5)
        if result.returncode == 0:
            version = result.stdout.split('\n')[0]
            print(f"✅ Tesseract found: {version}")
        else:
            tesseract_available = False
    except (FileNotFoundError, subprocess.TimeoutExpired):
        tesseract_available = False
    
    if not tesseract_available:
        issues.append("❌ Tesseract is not installed or not in PATH")
    
    # Check Python dependencies
    required_packages = [
        ('telegram', 'python-telegram-bot'),
        ('PIL', 'Pillow'),
        ('requests', 'requests'),
        ('pytesseract', 'pytesseract')
    ]
    
    for package, pip_name in required_packages:
        try:
            __import__(package)
            print(f"✅ {pip_name} is installed")
        except ImportError:
            issues.append(f"❌ {pip_name} is not installed. Install with: pip install {pip_name}")
    
    # Check for language packs (if tesseract is available)
    if tesseract_available:
        try:
            result = subprocess.run(['tesseract', '--list-langs'], 
                                  capture_output=True, text=True, timeout=5)
            if result.returncode == 0:
                langs = result.stdout.strip().split('\n')[1:]  # Skip first line
                supported_langs = ['eng', 'ara', 'deu', 'fra', 'spa', 'rus']
                available_langs = [lang for lang in supported_langs if lang in langs]
                missing_langs = [lang for lang in supported_langs if lang not in langs]
                
                if available_langs:
                    print(f"✅ Available language packs: {', '.join(available_langs)}")
                
                if missing_langs:
                    warnings.append(f"⚠️  Missing language packs: {', '.join(missing_langs)}")
                    warnings.append("   Install with: sudo apt install tesseract-ocr-<lang>")
        except subprocess.TimeoutExpired:
            warnings.append("⚠️  Could not check available language packs")
    
    # Check configuration files
    if os.path.exists('config.py'):
        print("✅ config.py found")
    else:
        warnings.append("⚠️  config.py not found. Using environment variables only.")
    
    if os.path.exists('.env'):
        print("✅ .env file found")
    
    # Summary
    print("\n" + "="*50)
    
    if not issues and not warnings:
        print("🎉 Configuration is perfect! You can run the bot now.")
        return True
    
    if warnings and not issues:
        print("✅ Configuration is good with minor warnings:")
        for warning in warnings:
            print(f"  {warning}")
        print("\nYou can run the bot, but some features might be limited.")
        return True
    
    if issues:
        print("❌ Configuration has issues that need to be fixed:")
        for issue in issues:
            print(f"  {issue}")
        
        if warnings:
            print("\nAdditional warnings:")
            for warning in warnings:
                print(f"  {warning}")
        
        print("\nPlease fix these issues before running the bot.")
        return False

if __name__ == '__main__':
    success = validate_config()
    sys.exit(0 if success else 1)