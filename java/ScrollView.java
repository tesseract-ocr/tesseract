// Copyright 2007 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); You may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by
// applicable law or agreed to in writing, software distributed under the
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

package com.google.scrollview;

import com.google.scrollview.events.SVEvent;
import com.google.scrollview.ui.SVImageHandler;

import org.keplerproject.luajava.LuaState;
import org.keplerproject.luajava.LuaStateFactory;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;


/**
 * The ScrollView class is the main class which gets started from the command
 * line. It sets up LUA and handles the network processing.
 * @author wanke@google.com
 */
public class ScrollView {

  /** The port our server listens at. */
  public static int SERVER_PORT = 8461;

  /**
   * All SVWindow objects share the same connection stream. The socket is needed
   * to detect when the connection got closed, in/out are used to send and
   * receive messages.
   */
  private static Socket socket;
  private static PrintStream out;
  public static BufferedReader in;
  public static float polylineXCoords[];  // The coords being received.
  public static float polylineYCoords[];  // The coords being received.
  public static int polylineSize;       // The size of the coords arrays.
  public static int polylineScanned;    // The size read so far.

  /** Keeps track of the number of messages received. */
  static int nrInputLines = 0;

  /** Binding to LUA */
  private static LuaState L;

  /** Prints all received messages to the console if true. */
  static boolean debugViewNetworkTraffic = false;

  /** Add a new message to the outgoing queue */
  public static void addMessage(SVEvent e) {
    if (debugViewNetworkTraffic) {
      System.out.println("(S->c) " + e.toString());
    }
    String str = e.toString();
    // Send the whole thing as UTF8.
    try {
      byte [] utf8 = str.getBytes("UTF8");
      out.write(utf8, 0, utf8.length);
    } catch (java.io.UnsupportedEncodingException ex) {
      System.out.println("Oops... can't encode to UTF8... Exiting");
      System.exit(0);
    }
    out.println();
    // Flush the output and check for errors.
    boolean error = out.checkError();
    if (error) {
      System.out.println("Connection error. Quitting ScrollView Server...");
      System.exit(0);
    }
  }

  /** Read one message from client (assuming there are any). */
  public static String receiveMessage() throws IOException {
    return in.readLine();
  }

  /**
   * The main program loop. Basically loops trough receiving messages and
   * processing them and then sending messages (if there are any).
   */
  private static void IOLoop() {
    String inputLine;

    try {
      while (!socket.isClosed() && !socket.isInputShutdown() &&
             !socket.isOutputShutdown() &&
             socket.isConnected() && socket.isBound()) {
        inputLine = receiveMessage();
        nrInputLines++;
        if (debugViewNetworkTraffic) {
          System.out.println("(c->S," + nrInputLines + ")" + inputLine);
        }

        if (polylineSize > polylineScanned) {
          // We are processing a polyline.
          // Read pairs of coordinates separated by commas.
          boolean first = true;
          for (String coordStr : inputLine.split(",")) {
            int coord = Integer.parseInt(coordStr);
            if (first) {
              polylineXCoords[polylineScanned] = coord;
            } else {
              polylineYCoords[polylineScanned++] = coord;
            }
            first = !first;
          }
          assert first;
        } else if (SVImageHandler.getReadImageData() == false) {
          // If we are currently not transmitting an image, process this
          // normally.
          processInput(inputLine);
        }
        // We are still transmitting image data, but there seems to be some
        // command at the
        // end of the message attached as well. Thus, we have to split it
        // accordingly and
        // first generate the image and afterwards process the remaining
        // message.
        else if (inputLine.length() >
                 SVImageHandler.getMissingRemainingBytes()) {
          String luaCmd = inputLine.substring(
              SVImageHandler.getMissingRemainingBytes());
          String imgData = inputLine.substring(0,
              SVImageHandler.getMissingRemainingBytes());
          SVImageHandler.parseData(imgData);
          processInput(luaCmd);
        } else { // We are still in the middle of image data and have not
                 // reached the end yet.
          SVImageHandler.parseData(inputLine);
        }
      }
    }
    // Some connection error
    catch (IOException e) {
      System.out.println("Connection error. Quitting ScrollView Server...");
    }
    System.exit(0);
  }

  /** Executes the LUA command parsed as parameter. */
  private static void processInput(String inputLine) {
    int err = L.LdoString(inputLine);
    if (err == 1) {
      System.out
          .println("LUA Error in:" + inputLine + "(" + nrInputLines + ")");
    }
  }

  /** Called from the client to make the server exit. */
  public static void exit() {
    System.exit(0);
  }

  /**
   * The main function. Sets up LUA and the server connection and then calls the
   * IOLoop.
   */
  public static void main(String[] args) {
    if (args.length > 0) {
      SERVER_PORT = Integer.parseInt(args[0]);
    }
    L = LuaStateFactory.newLuaState();
    L.openLibs();

    try {
      // Open a socket to listen on.
      ServerSocket serverSocket = new ServerSocket(SERVER_PORT);
      System.out.println("Socket started on port " + SERVER_PORT);

      // Wait (blocking) for an incoming connection
      socket = serverSocket.accept();
      System.out.println("Client connected");

      // Setup the streams
      out = new PrintStream(socket.getOutputStream(), true);
      in =
          new BufferedReader(new InputStreamReader(socket.getInputStream(),
              "UTF8"));
    } catch (IOException e) {
      // Something went wrong and we were unable to set up a connection. This is
      // pretty
      // much a fatal error.
      // Note: The server does not get restarted automatically if this happens.
      e.printStackTrace();
      System.exit(1);
    }

    // Enter the main program loop.
    IOLoop();
  }
}
