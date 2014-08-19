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
import com.google.scrollview.ui.SVWindow;
import org.piccolo2d.nodes.PImage;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.regex.Pattern;

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
  private static ArrayList<SVWindow> windows;  // The id to SVWindow map.
  private static Pattern intPattern;        // For checking integer arguments.
  private static Pattern floatPattern;     // For checking float arguments.

  /** Keeps track of the number of messages received. */
  static int nrInputLines = 0;

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
        } else {
          // Process this normally.
          processInput(inputLine);
        }
      }
    }
    // Some connection error
    catch (IOException e) {
      System.out.println("Connection error. Quitting ScrollView Server...");
    }
    System.exit(0);
  }

  // Parse a comma-separated list of arguments into ArrayLists of the
  // possible types. Each type is stored in order, but the order
  // distinction between types is lost.
  // Note that the format is highly constrained to what the client used
  // to send to LUA:
  // Quoted string -> String.
  // true or false -> Boolean.
  // %f format number -> Float (no %e allowed)
  // Sequence of digits -> Integer
  // Nothing else allowed.
  private static void parseArguments(String argList,
                                     ArrayList<Integer> intList,
                                     ArrayList<Float> floatList,
                                     ArrayList<String> stringList,
                                     ArrayList<Boolean> boolList) {
    // str is only non-null if an argument starts with a single or double
    // quote. str is set back to null on completion of the string with a
    // matching quote. If the string contains a comma then str will stay
    // non-null across multiple argStr values until a matching closing quote.
    // Backslash escaped quotes do not count as terminating the string.
    String str = null;
    for (String argStr : argList.split(",")) {
      if (str != null) {
        // Last string was incomplete. Append argStr to it and restore comma.
        // Execute str += "," + argStr in Java.
        int length = str.length() + 1 + argStr.length();
        StringBuilder appended = new StringBuilder(length);
        appended.append(str);
        appended.append(",");
        appended.append(argStr);
        str =  appended.toString();
      } else if (argStr.length() == 0) {
        continue;
      } else {
        char quote = argStr.charAt(0);
        // If it begins with a quote then it is a string, but may not
        // end this time if it contained a comma.
        if (quote == '\'' || quote == '"') {
          str = argStr;
        }
      }
      if (str != null) {
        // It began with a quote. Check that it still does.
        assert str.charAt(0) == '\'' || str.charAt(0) == '"';
        int len = str.length();
        if (len > 1 && str.charAt(len - 1) == str.charAt(0)) {
          // We have an ending quote of the right type. Now check that
          // it is not escaped. Must have an even number of slashes before.
          int slash = len - 1;
          while (slash > 0 && str.charAt(slash - 1) == '\\')
            --slash;
          if ((len - 1 - slash) % 2 == 0) {
            // It is now complete. Chop off the quotes and save.
            // TODO(rays) remove the first backslash of each pair.
            stringList.add(str.substring(1, len - 1));
            str = null;
          }
        }
        // If str is not null here, then we have a string with a comma in it.
        // Append , and the next argument at the next iteration, but check
        // that str is null after the loop terminates in case it was an
        // unterminated string.
      } else if (floatPattern.matcher(argStr).matches()) {
        // It is a float.
        floatList.add(Float.parseFloat(argStr));
      } else if (argStr.equals("true")) {
        boolList.add(true);
      } else if (argStr.equals("false")) {
        boolList.add(false);
      } else if (intPattern.matcher(argStr).matches()) {
        // Only contains digits so must be an int.
        intList.add(Integer.parseInt(argStr));
      }
      // else ignore all incompatible arguments for forward compatibility.
    }
    // All strings must have been terminated.
    assert str == null;
  }

  /** Executes the LUA command parsed as parameter. */
  private static void processInput(String inputLine) {
    if (inputLine == null) {
      return;
    }
    // Execute a function encoded as a LUA statement! Yuk!
    if (inputLine.charAt(0) == 'w') {
      // This is a method call on a window. Parse it.
      String noWLine = inputLine.substring(1);
      String[] idStrs = noWLine.split("[ :]", 2);
      int windowID = Integer.parseInt(idStrs[0]);
      // Find the parentheses.
      int start = inputLine.indexOf('(');
      int end = inputLine.lastIndexOf(')');
      // Parse the args.
      ArrayList<Integer> intList = new ArrayList<Integer>(4);
      ArrayList<Float> floatList = new ArrayList<Float>(2);
      ArrayList<String> stringList = new ArrayList<String>(4);
      ArrayList<Boolean> boolList = new ArrayList<Boolean>(3);
      parseArguments(inputLine.substring(start + 1, end),
                     intList, floatList, stringList, boolList);
      int colon = inputLine.indexOf(':');
      if (colon > 1 && colon < start) {
        // This is a regular function call. Look for the name and call it.
        String func = inputLine.substring(colon + 1, start);
        if (func.equals("drawLine")) {
          windows.get(windowID).drawLine(intList.get(0), intList.get(1),
                                         intList.get(2), intList.get(3));
        } else if (func.equals("createPolyline")) {
          windows.get(windowID).createPolyline(intList.get(0));
        } else if (func.equals("drawPolyline")) {
          windows.get(windowID).drawPolyline();
        } else if (func.equals("drawRectangle")) {
          windows.get(windowID).drawRectangle(intList.get(0), intList.get(1),
                                              intList.get(2), intList.get(3));
        } else if (func.equals("setVisible")) {
          windows.get(windowID).setVisible(boolList.get(0));
        } else if (func.equals("setAlwaysOnTop")) {
          windows.get(windowID).setAlwaysOnTop(boolList.get(0));
        } else if (func.equals("addMessage")) {
          windows.get(windowID).addMessage(stringList.get(0));
        } else if (func.equals("addMessageBox")) {
          windows.get(windowID).addMessageBox();
        } else if (func.equals("clear")) {
          windows.get(windowID).clear();
        } else if (func.equals("setStrokeWidth")) {
          windows.get(windowID).setStrokeWidth(floatList.get(0));
        } else if (func.equals("drawEllipse")) {
          windows.get(windowID).drawEllipse(intList.get(0), intList.get(1),
                                            intList.get(2), intList.get(3));
        } else if (func.equals("pen")) {
          if (intList.size() == 4) {
            windows.get(windowID).pen(intList.get(0), intList.get(1),
                                      intList.get(2), intList.get(3));
          } else {
            windows.get(windowID).pen(intList.get(0), intList.get(1),
                                      intList.get(2));
          }
        } else if (func.equals("brush")) {
          if (intList.size() == 4) {
            windows.get(windowID).brush(intList.get(0), intList.get(1),
                                        intList.get(2), intList.get(3));
          } else {
            windows.get(windowID).brush(intList.get(0), intList.get(1),
                                        intList.get(2));
          }
        } else if (func.equals("textAttributes")) {
          windows.get(windowID).textAttributes(stringList.get(0),
                                               intList.get(0),
                                               boolList.get(0),
                                               boolList.get(1),
                                               boolList.get(2));
        } else if (func.equals("drawText")) {
          windows.get(windowID).drawText(intList.get(0), intList.get(1),
                                         stringList.get(0));
        } else if (func.equals("addMenuBarItem")) {
          if (boolList.size() > 0) {
            windows.get(windowID).addMenuBarItem(stringList.get(0),
                                                 stringList.get(1),
                                                 intList.get(0),
                                                 boolList.get(0));
          } else if (intList.size() > 0) {
            windows.get(windowID).addMenuBarItem(stringList.get(0),
                                                 stringList.get(1),
                                                 intList.get(0));
          } else {
            windows.get(windowID).addMenuBarItem(stringList.get(0),
                                                 stringList.get(1));
          }
        } else if (func.equals("addPopupMenuItem")) {
          if (stringList.size() == 4) {
            windows.get(windowID).addPopupMenuItem(stringList.get(0),
                                                   stringList.get(1),
                                                   intList.get(0),
                                                   stringList.get(2),
                                                   stringList.get(3));
          } else {
             windows.get(windowID).addPopupMenuItem(stringList.get(0),
                                                    stringList.get(1));
          }
        } else if (func.equals("update")) {
          windows.get(windowID).update();
        } else if (func.equals("showInputDialog")) {
          windows.get(windowID).showInputDialog(stringList.get(0));
        } else if (func.equals("showYesNoDialog")) {
          windows.get(windowID).showYesNoDialog(stringList.get(0));
        } else if (func.equals("zoomRectangle")) {
          windows.get(windowID).zoomRectangle(intList.get(0), intList.get(1),
                                              intList.get(2), intList.get(3));
        } else if (func.equals("readImage")) {
          PImage image = SVImageHandler.readImage(intList.get(2), in);
          windows.get(windowID).drawImage(image, intList.get(0), intList.get(1));
        } else if (func.equals("drawImage")) {
          PImage image = new PImage(stringList.get(0));
          windows.get(windowID).drawImage(image, intList.get(0), intList.get(1));
        } else if (func.equals("destroy")) {
          windows.get(windowID).destroy();
        }
        // else for forward compatibility purposes, silently ignore any
        // unrecognized function call.
      } else {
        // No colon. Check for create window.
        if (idStrs[1].startsWith("= luajava.newInstance")) {
          while (windows.size() <= windowID) {
            windows.add(null);
          }
          windows.set(windowID, new SVWindow(stringList.get(1),
                                             intList.get(0), intList.get(1),
                                             intList.get(2), intList.get(3),
                                             intList.get(4), intList.get(5),
                                             intList.get(6)));
        }
        // else for forward compatibility purposes, silently ignore any
        // unrecognized function call.
      }
    } else if (inputLine.startsWith("svmain")) {
        // Startup or end. Startup is a lua bind, which is now a no-op.
        if (inputLine.startsWith("svmain:exit")) {
          exit();
        }
        // else for forward compatibility purposes, silently ignore any
        // unrecognized function call.
    }
    // else for forward compatibility purposes, silently ignore any
    // unrecognized function call.
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
    windows = new ArrayList<SVWindow>(100);
    intPattern = Pattern.compile("[0-9-][0-9]*");
    floatPattern = Pattern.compile("[0-9-][0-9]*\\.[0-9]*");

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
