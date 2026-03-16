import os
from flask import Flask, render_template, request, flash, redirect, url_for
from werkzeug.utils import secure_filename
import pytesseract
from PIL import Image, ImageEnhance, ImageFilter

app = Flask(__name__)
app.secret_key = os.environ.get('SECRET_KEY', 'default_dev_secret_key_12345')

# Setup upload folder
UPLOAD_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__name__)), 'uploads')
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# Ensure allowable file types
ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg', 'gif', 'webp', 'bmp', 'tiff'}

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def preprocess_image(image_path):
    """
    Preprocess the image to enhance handwriting recognition.
    """
    img = Image.open(image_path)
    # Convert to grayscale
    img = img.convert('L')

    # Enhance contrast
    enhancer = ImageEnhance.Contrast(img)
    img = enhancer.enhance(2.0)

    # Apply a slight median filter to remove noise
    img = img.filter(ImageFilter.MedianFilter(size=3))

    return img

@app.route('/', methods=['GET', 'POST'])
def index():
    extracted_text = None
    error = None

    if request.method == 'POST':
        # check if the post request has the file part
        if 'file' not in request.files:
            flash('No file part')
            return redirect(request.url)
        file = request.files['file']

        # If the user does not select a file, the browser submits an
        # empty file without a filename.
        if file.filename == '':
            flash('No selected file')
            return redirect(request.url)

        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(filepath)

            # Retrieve parameters for OCR
            lang = request.form.get('lang', 'eng')
            ocr_mode = request.form.get('mode', 'printed')

            try:
                # Preprocess image
                img = preprocess_image(filepath)

                # Perform OCR
                if ocr_mode == 'handwriting':
                    # Custom configuration to help with handwriting:
                    # --psm 6 (Assume a single uniform block of text)
                    # --oem 1 (Neural nets LSTM only - better for handwriting)
                    custom_config = r'--oem 1 --psm 6'
                else:
                    # Default for printed text
                    custom_config = r'--oem 3 --psm 3'

                extracted_text = pytesseract.image_to_string(img, lang=lang, config=custom_config)
            except Exception as e:
                error = f"Error during OCR processing: {e}"
            finally:
                # Clean up the file
                if os.path.exists(filepath):
                    os.remove(filepath)
        else:
            flash('Invalid file type. Allowed: png, jpg, jpeg, gif, webp, bmp, tiff')
            return redirect(request.url)

    return render_template('index.html', extracted_text=extracted_text, error=error)

if __name__ == '__main__':
    # Determine port for Render
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=False)