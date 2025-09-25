#!/usr/bin/env python3
"""
Test runner for Tesseract Telegram Bot
Runs all available tests to validate the bot setup.
"""

import subprocess
import sys
import os

def run_test(test_name, test_file):
    """Run a single test and return the result"""
    print(f"\n{'='*50}")
    print(f"Running {test_name}...")
    print('='*50)
    
    try:
        result = subprocess.run([sys.executable, test_file], 
                              capture_output=False, 
                              text=True, 
                              timeout=30)
        return result.returncode == 0
    except subprocess.TimeoutExpired:
        print(f"❌ {test_name} timed out")
        return False
    except Exception as e:
        print(f"❌ {test_name} failed with error: {e}")
        return False

def main():
    """Run all tests"""
    print("🧪 Running Tesseract Telegram Bot Test Suite")
    
    # Change to the correct directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    
    tests = [
        ("Syntax Validation", "test_bot_token.py"),
        ("Configuration Validation", "validate_config.py"),
    ]
    
    results = []
    
    for test_name, test_file in tests:
        if os.path.exists(test_file):
            success = run_test(test_name, test_file)
            results.append((test_name, success))
        else:
            print(f"⚠️  Test file {test_file} not found, skipping {test_name}")
            results.append((test_name, None))
    
    # Summary
    print(f"\n{'='*50}")
    print("TEST SUMMARY")
    print('='*50)
    
    passed = 0
    failed = 0
    skipped = 0
    
    for test_name, result in results:
        if result is True:
            print(f"✅ {test_name}: PASSED")
            passed += 1
        elif result is False:
            print(f"❌ {test_name}: FAILED")
            failed += 1
        else:
            print(f"⚠️  {test_name}: SKIPPED")
            skipped += 1
    
    print(f"\nResults: {passed} passed, {failed} failed, {skipped} skipped")
    
    if failed > 0:
        print("\n❌ Some tests failed. Please check the output above.")
        return False
    elif passed == 0:
        print("\n⚠️  No tests were run successfully.")
        return False
    else:
        print(f"\n🎉 All {passed} tests passed!")
        return True

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)