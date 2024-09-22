import { NextRequest, NextResponse } from 'next/server';
import { writeFile } from 'fs/promises';
import { exec } from 'child_process';
import { promisify } from 'util';
import path from 'path';
import os from 'os';

const execAsync = promisify(exec);

export async function POST(request: NextRequest) {
  try {
    const formData = await request.formData();
    const image = formData.get('image') as File;

    if (!image) {
      return NextResponse.json({ error: 'No image uploaded' }, { status: 400 });
    }

    // Create a temporary file path
    const tempDir = os.tmpdir();
    const fileName = `${Date.now()}_${image.name}`;
    const filePath = path.join(tempDir, fileName);

    // Write the file to the temporary directory
    const bytes = await image.arrayBuffer();
    const buffer = Buffer.from(bytes);
    await writeFile(filePath, buffer);

    // Run Tesseract OCR
    const { stdout, stderr } = await execAsync(`tesseract "${filePath}" stdout -l khm --psm 1`);

    // Clean up the temporary file
    await execAsync(`rm "${filePath}"`);

    if (stderr) {
      console.error('Tesseract Error:', stderr);
      return NextResponse.json({ text: stdout.trim(), error: stderr }, { status: 200 });
    }

    return NextResponse.json({ text: stdout.trim() });
  } catch (error) {
    console.error('OCR Error:', error);
    return NextResponse.json({ error: 'Error processing image' }, { status: 500 });
  }
}