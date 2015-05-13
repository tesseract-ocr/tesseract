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

import org.piccolo2d.nodes.PImage;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import javax.imageio.ImageIO;
import javax.xml.bind.DatatypeConverter;

/**
 * The ScrollViewImageHandler is a helper class which takes care of image
 * processing. It is used to construct an Image from the message-stream and
 * basically consists of a number of utility functions to process the input
 * stream.
 *
 * @author wanke@google.com
 */
public class SVImageHandler {
  /* All methods are static, so we forbid to construct SVImageHandler objects */
  private SVImageHandler() {
  }

  /**
   * Reads size bytes from the stream in and interprets it as an image file,
   * encoded as png, and then text-encoded as base 64, returning the decoded
   * bitmap.
   *
   * @param size The size of the image file.
   * @param in The input stream from which to read the bytes.
   */
  public static PImage readImage(int size, BufferedReader in) {
    char[] charbuffer = new char[size];
    int numRead = 0;
    while (numRead < size) {
      int newRead = -1;
      try {
        newRead = in.read(charbuffer, numRead, size - numRead);
      } catch (IOException e) {
        System.out.println("Failed to read image data from socket:" + e.getMessage());
        return null;
      }
      if (newRead < 0) {
        return null;
      }
      numRead += newRead;
    }
    if (numRead != size) {
        System.out.println("Failed to read image data from socket");
      return null;
    }
    // Convert the character data to binary.
    byte[] binarydata = DatatypeConverter.parseBase64Binary(new String(charbuffer));
    // Convert the binary data to a byte stream and parse to image.
    ByteArrayInputStream byteStream = new ByteArrayInputStream(binarydata);
    try {
      PImage img = new PImage(ImageIO.read(byteStream));
      return img;
    } catch (IOException e) {
      System.out.println("Failed to decode image data from socket" + e.getMessage());
    }
    return null;
  }
}
