// Copyright 2007 Google Inc. All Rights Reserved.
// 
// Licensed under the Apache License, Version 2.0 (the "License"); You may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by
// applicable law or agreed to in writing, software distributed under the
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

package com.google.scrollview.ui;

import edu.umd.cs.piccolo.nodes.PImage;

import java.awt.image.BufferedImage;
import java.util.HashMap;

/**
 * The ScrollViewImageHandler is a helper class which takes care of image
 * processing. It is used to construct an Image from the message-stream and
 * basically consists of a number of utility functions to process the input
 * stream.
 * 
 * @author wanke@google.com
 */
public class SVImageHandler {
  /**
   * Stores a mapping from the name of the string to its actual image. It
   * enables us to re-use images without having to load or transmit them again
   */
  static HashMap<String, PImage> images = new HashMap<String, PImage>();

  /** A global flag stating whether we are currently expecting Image data */
  static boolean readImageData = false;

  // TODO(wanke) Consider moving all this into an SVImage class.
  /** These are all values belonging to the image which is currently being read */
  static String imageName = null; // Image name
  static int bytesRead = 0; // Nr. of bytes already read
  static int bpp = 0; // Bit depth
  static int pictureArray[]; // The array holding the actual image

  static int bytePerPixel = 0; // # of used bytes to transmit a pixel (32 bpp
                                // -> 7 BPP)
  static int width = 0;
  static int height = 0;

  /* All methods are static, so we forbid to construct SVImageHandler objects */
  private SVImageHandler() {
  }

  /**
   * Takes a binary input string (consisting of '0' and '1' characters) and
   * converts it to an integer representation usable as image data.
   */
  private static int[] processBinaryImage(String inputLine) {
    int BLACK = 0;
    int WHITE = Integer.MAX_VALUE;

    int[] imgData = new int[inputLine.length()];

    for (int i = 0; i < inputLine.length(); i++) {
      if (inputLine.charAt(i) == '0') {
        imgData[i] = WHITE;
      } else if (inputLine.charAt(i) == '1') {
        imgData[i] = BLACK;
      } // BLACK is default anyway
      else { // Something is wrong: We did get unexpected data
        System.out.println("Error: unexpected non-image-data: ("
            + SVImageHandler.bytesRead + "," + inputLine.length() + ","
            + (SVImageHandler.height * SVImageHandler.width) + ")");
        System.exit(1);
      }
    }
    return imgData;
  }

  /**
   * Takes an input string with pixel depth of 8 (represented by 2 bytes in
   * hexadecimal format, e.g. FF for white) and converts it to an
   * integer representation usable as image data
   */
  private static int[] processGrayImage(String inputLine) {
    int[] imgData = new int[inputLine.length() / 2];
    // Note: This is really inefficient, splitting it 2-byte-arrays in one pass
    // would be wa faster than substring everytime.
    for (int i = 0; i < inputLine.length(); i +=2) {
      String s = inputLine.substring(i, i+1);
      imgData[i] = Integer.parseInt(s, 16);
    }
    
    return imgData;
  }
  
  /**
   * Takes an input string with pixel depth of 32 (represented by HTML-like
   * colors in hexadecimal format, e.g. #00FF00 for green) and converts it to an
   * integer representation usable as image data
   */
  private static int[] process32bppImage(String inputLine) {

    String[] strData = inputLine.split("#");
    int[] imgData = new int[strData.length - 1];

    for (int i = 1; i < strData.length; i++) {
      imgData[i - 1] = Integer.parseInt(strData[i], 16);
    }

    return imgData;
  }

  /**
   * Called when all image data is transmitted. Generates the actual image used
   * by java and puts it into the images-hashmap.
   */
  private static void closeImage() {

    BufferedImage bi = null;
    if (bpp == 1) {
      bi = new BufferedImage(width, height, BufferedImage.TYPE_BYTE_BINARY);
    } else if (bpp == 8) {
      bi = new BufferedImage(width, height, BufferedImage.TYPE_BYTE_GRAY);
    } else if (bpp == 32) {
      bi = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
    } else {
      System.out.println("Unsupported Image Type: " + bpp + " bpp");
      System.exit(1);
    }

    bi.setRGB(0, 0, width, height, pictureArray, 0, width);

    PImage img = new PImage(bi);

    images.put(imageName, img);

    imageName = null;
    readImageData = false;

    System.out.println("(server, #Bytes:" + bytesRead + ") Image Completed");

    bytesRead = 0;
    bpp = 0;
  }

  /** Starts creation of a new image. */
  public static void createImage(String name, int width, int height,
      int bitsPerPixel) {
    // Create buffered image that does not support transparency
    bpp = bitsPerPixel;
    if (bpp == 1) {
      bytePerPixel = 1;
    } else if (bpp == 8) {
      bytePerPixel = 2;
    } else if (bpp == 32) {
      bytePerPixel = 7;
    } else {
      throw new IllegalArgumentException(
          "bpp should be 1 (binary), 8 (gray) or 32 (argb), is " + bpp);
    }
    if (imageName != null) {
      throw new IllegalArgumentException("Image " + imageName + " already opened!");
    }
    else {
      imageName = name;
      bytesRead = 0;
      readImageData = true;
      SVImageHandler.height = height;
      SVImageHandler.width = width;
      pictureArray = new int[width * height];
    }   

    System.out.println("Processing Image with " + bpp + " bpp, size " + width + "x" + height);
  }

  /**
   * Opens an Image from location. This means the image does not have to be
   * actually transfered over the network. Thus, it is a lot faster than using
   * the createImage method.
   * 
   * @param location The (local) location from where to open the file. This is
   * also the internal name associated with the image (if you want to draw it).
   */
  public static void openImage(String location) {
    PImage img = new PImage(location);
    images.put(location, img);
  }

  /** Find the image corresponding to a given name */
  public static PImage getImage(String name) {
    return images.get(name);
  }

  /**
   * Gets called while currently reading image data. Decides, how to process it
   * (which image type, whether all data is there).
   */
  public static void parseData(String inputLine) {
    int[] data = null;

    if (bpp == 1) {
      data = processBinaryImage(inputLine);
    } else if (bpp == 8) {
      data = processGrayImage(inputLine);
    } else if (bpp == 32) {
      data = process32bppImage(inputLine);
    } else {
      System.out.println("Unsupported Bit Type: " + bpp);
    }

    System.arraycopy(data, 0, pictureArray, bytesRead, data.length);
    bytesRead += data.length;

    // We have read all image data - close the image
    if (bytesRead == (height * width)) {
      closeImage();
    }
  }

  /** Returns whether we a currently reading image data or not */
  public static boolean getReadImageData() {
    return readImageData;
  }

  /** Computes how many bytes of the image data are still missing */
  public static int getMissingRemainingBytes() {
    return (height * width * bytePerPixel) - bytesRead;
  }
}
