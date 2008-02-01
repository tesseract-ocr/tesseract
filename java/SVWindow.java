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

import com.google.scrollview.ScrollView;
import com.google.scrollview.events.SVEvent;
import com.google.scrollview.events.SVEventHandler;
import com.google.scrollview.events.SVEventType;
import com.google.scrollview.ui.SVImageHandler;
import com.google.scrollview.ui.SVMenuBar;
import com.google.scrollview.ui.SVPopupMenu;

import edu.umd.cs.piccolo.PCanvas;
import edu.umd.cs.piccolo.PLayer;

import edu.umd.cs.piccolo.nodes.PImage;
import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.nodes.PText;
import edu.umd.cs.piccolox.swing.PScrollPane;
import edu.umd.cs.piccolox.util.PFixedWidthStroke;

import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Font;
import java.awt.GraphicsEnvironment;
import java.awt.geom.IllegalPathStateException;
import java.awt.Rectangle;
import java.awt.TextArea;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import javax.swing.WindowConstants;

/**
 * The SVWindow is the top-level ui class. It should get instantiated whenever
 * the user intends to create a new window. It contains helper functions to draw
 * on the canvas, add new menu items, show modal dialogs etc.
 *
 * @author wanke@google.com
 */
public class SVWindow extends JFrame {
  /**
   * Constants defining the maximum initial size of the window.
   */
  private static final int MAX_WINDOW_X = 800;
  private static final int MAX_WINDOW_Y = 1000;

  /** Constant defining the "speed" at which to zoom in and out. */
  public static final double SCALING_FACTOR = 2;

  /** The top level layer we add our PNodes to (root node). */
  PLayer layer;

  /** The current color of the pen. It is used to draw edges, text, etc. */
  Color currentPenColor;

  /**
   * The current color of the brush. It is used to draw the interior of
   * primitives.
   */
  Color currentBrushColor;

  /** The system name of the current font we are using (e.g. "Times New Roman"). */
  Font currentFont;

  // TODO(wanke) allow user to change this, as it might be somewhat unexpected
  // default behaviour.
  /** The stroke width to be used. */
  // This really needs to be a fixed width stroke as the basic stroke is
  // anti-aliased and gets too faint, but the piccolo fixed width stroke
  // is too buggy and generates missing initial moveto in path definition
  // errors with a IllegalPathStateException that cannot be caught because
  // it is in the automatic repaint function.
  // PFixedWidthStroke stroke = new PFixedWidthStroke(1.0f);
  BasicStroke stroke = new BasicStroke(0.75f);

  /**
   * A unique representation for the window, also known by the client. It is
   * used when sending messages from server to client to identify him.
   */
  public int hash;

  /**
   * The total number of created Windows. If this ever reaches 0 (apart from the
   * beginning), quit the server.
   */
  public static int nrWindows = 0;

  /**
   * The Canvas, MessageBox, EventHandler, Menubar and Popupmenu associated with
   * this window.
   */
  private SVEventHandler svEventHandler = null;
  private SVMenuBar svMenuBar = null;
  private TextArea ta = null;
  public SVPopupMenu svPuMenu = null;
  public PCanvas canvas;

  /** Set the brush to an RGB color */
  public void brush(int red, int green, int blue) {
    brush(red, green, blue, 255);
  }

  /** Set the brush to an RGBA color */
  public void brush(int red, int green, int blue, int alpha) {
    if (red == -1) {
      currentBrushColor = new Color(0, 0, 0, 0);
    } else {
      currentBrushColor = new Color(red, green, blue, alpha);
    }
  }

  /** Erase all content from the window, but do not destroy it. */
  public void clear() {
    layer.removeAllChildren();
  }

  /**
   * Start setting up a new image. The server will now expect image data until
   * the image is complete.
   *
   * @param internalName The unique name of the new image
   * @param width Image width
   * @param height Image height
   * @param bitsPerPixel The bit depth (currently supported: 1 (binary) and 32
   *        (ARGB))
   */
  public void createImage(String internalName, int width, int height,
      int bitsPerPixel) {
    SVImageHandler.createImage(internalName, width, height, bitsPerPixel);
  }

  /**
   * Construct a new SVWindow and set it visible.
   *
   * @param name Title of the window.
   * @param hash Unique internal representation. This has to be the same as
   *        defined by the client, as they use this to refer to the windows.
   * @param posX X position of where to draw the window (upper left).
   * @param posY Y position of where to draw the window (upper left).
   * @param sizeX The width of the window.
   * @param sizeY The height of the window.
   * @param canvasSizeX The canvas width of the window.
   * @param canvasSizeY The canvas height of the window.
   */
  public SVWindow(String name, int hash, int posX, int posY, int sizeX,
      int sizeY, int canvasSizeX, int canvasSizeY) {

    super(name);

    // Initialize variables
    nrWindows++;
    this.hash = hash;
    this.svEventHandler = new SVEventHandler(this);
    this.currentPenColor = Color.BLACK;
    this.currentBrushColor = Color.BLACK;
    this.currentFont = new Font("Times New Roman", Font.PLAIN, 12);

    // Determine the initial size and zoom factor of the window.
    // If the window is too big, rescale it and zoom out.
    float initialScalingfactor = 1;
    int winSizeX = Math.max(canvasSizeX, sizeX);
    int winSizeY = Math.max(canvasSizeY, sizeY);

    if (winSizeX > MAX_WINDOW_X) {
      initialScalingfactor = MAX_WINDOW_X / winSizeX;
      winSizeX = MAX_WINDOW_X;
    }
    if (winSizeY > MAX_WINDOW_Y) {
      if ((MAX_WINDOW_Y / winSizeY) < initialScalingfactor) {
        initialScalingfactor = MAX_WINDOW_Y / winSizeY;
      }
      winSizeY = MAX_WINDOW_Y;
    }

    // Setup the actual window (its size, camera, title, etc.)
    if (canvas == null) {
      canvas = new PCanvas();
      getContentPane().add(canvas, BorderLayout.CENTER);
    }

    layer = canvas.getLayer();
    canvas.setBackground(Color.BLACK);

    setLayout(new BorderLayout());

    setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);

    validate();
    canvas.requestFocus();

    // Manipulation of Piccolo's scene graph should be done from Swings
    // event dispatch thread since Piccolo is not thread safe. This code calls
    // initialize() from that thread once the PFrame is initialized, so you are
    // safe to start working with Piccolo in the initialize() method.
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        repaint();
      }
    });

    setSize(winSizeX, winSizeY);
    canvas.getCamera().setViewBounds(
        new Rectangle(0, 0, Math.max(canvasSizeX, sizeX), Math.max(canvasSizeY,
            sizeY)));
    setLocation(posX, posY);
    setTitle(name);

    // Add a Scrollpane to be able to scroll within the canvas
    PScrollPane scrollPane = new PScrollPane(canvas);
    getContentPane().add(scrollPane);
    scrollPane.setWheelScrollingEnabled(false);

    // Disable the default event handlers and add our own.
    addWindowListener(svEventHandler);
    canvas.removeInputEventListener(canvas.getPanEventHandler());
    canvas.removeInputEventListener(canvas.getZoomEventHandler());
    canvas.addInputEventListener(svEventHandler);
    canvas.addKeyListener(svEventHandler);

    // Make the window visible.
    validate();
    setVisible(true);

  }

  /**
   * Convenience function to add a message box to the window which can be used
   * to output debug information.
   */
  public void addMessageBox() {
    if (ta == null) {
      ta = new TextArea();
      ta.setEditable(false);
      getContentPane().add(ta, BorderLayout.SOUTH);
    }
  }

  /**
   * Allows you to specify the thickness with which to draw lines, recantgles and ellipses.
   * @param width The new thickness.
   */
  public void setStrokeWidth(float width) {
//      stroke = new PFixedWidthStroke(width);
    stroke = new BasicStroke(width);
  }

  /**
   * Draw an ellipse at (x,y) with given width and height, using the current stroke,
   * the current brush color to fill it and the current pen color for the outline.
   */
  public void drawEllipse(int x, int y, int width, int height) {
    PPath pn = PPath.createEllipse(x, y, width, height);
    pn.setStrokePaint(currentPenColor);
    pn.setStroke(stroke);
    pn.setPaint(currentBrushColor);
    layer.addChild(pn);
  }

  /**
   * Draw the image with the given name at (x,y). Any image loaded stays in
   * memory, so if you intend to redraw an image, you do not have to use
   * createImage again.
   */
  public void drawImage(String internalName, int x_pos, int y_pos) {
    PImage img = SVImageHandler.getImage(internalName);
    img.setX(x_pos);
    img.setY(y_pos);
    layer.addChild(img);
  }

  /**
   * Draw a line from (x1,y1) to (x2,y2) using the current pen color and stroke.
   */
  public void drawLine(int x1, int y1, int x2, int y2) {
    PPath pn = PPath.createLine(x1, y1, x2, y2);
    pn.setStrokePaint(currentPenColor);
    pn.setStroke(stroke);
    pn.moveTo(x1, y1);
    pn.lineTo(x2, y2);
    layer.addChild(pn);
  }

  /**
   * Draw a rectangle given the two points (x1,y1) and (x2,y2) using the current stroke,
   * pen color for the border and the brush to fill the interior.
   */
  public void drawRectangle(int x1, int y1, int x2, int y2) {

    if (x1 > x2) {
      int t = x1;
      x1 = x2;
      x2 = t;
    }
    if (y1 > y2) {
      int t = y1;
      y1 = y2;
      y2 = t;
    }

    PPath pn = PPath.createRectangle(x1, y1, x2 - x1, y2 - y1);
    pn.setStrokePaint(currentPenColor);
    pn.setStroke(stroke);
    pn.setPaint(currentBrushColor);
    layer.addChild(pn);
  }

  /**
   * Draw some text at (x,y) using the current pen color and text attributes. If
   * the current font does NOT support at least one character, it tries to find
   * a font which is capable of displaying it and use that to render the text.
   * Note: If the font says it can render a glyph, but in reality it turns out
   * to be crap, there is nothing we can do about it.
   */
  public void drawText(int x, int y, String text) {
    int unreadableCharAt = -1;
    char[] chars = text.toCharArray();
    PText pt = new PText(text);
    pt.setTextPaint(currentPenColor);
    pt.setFont(currentFont);

    // Check to see if every character can be displayed by the current font.
    for (int i = 0; i < chars.length; i++) {
      if (!currentFont.canDisplay(chars[i])) {
        // Set to the first not displayable character.
        unreadableCharAt = i;
        break;
      }
    }

    // Have to find some working font and use it for this text entry.
    if (unreadableCharAt != -1) {
      Font[] allfonts =
          GraphicsEnvironment.getLocalGraphicsEnvironment().getAllFonts();
      for (int j = 0; j < allfonts.length; j++) {
        if (allfonts[j].canDisplay(chars[unreadableCharAt])) {
          Font tempFont =
              new Font(allfonts[j].getFontName(), currentFont.getStyle(),
                  currentFont.getSize());
          pt.setFont(tempFont);
          break;
        }
      }
    }

    pt.setX(x);
    pt.setY(y);
    layer.addChild(pt);
  }

  /** Set the pen color to an RGB value */
  public void pen(int red, int green, int blue) {
    pen(red, green, blue, 255);
  }

  /** Set the pen color to an RGBA value */
  public void pen(int red, int green, int blue, int alpha) {
    if (red == -1) {
      currentPenColor = new Color(0, 0, 0, 0);
    } else {
      currentPenColor = new Color(red, green, blue, alpha);
    }
  }

  /**
   * Define how to display text. Note: underlined is not currently not supported
   */
  public void textAttributes(String font, int pixelSize, boolean bold,
      boolean italic, boolean underlined) {

    // For legacy reasons convert "Times" to "Times New Roman"
    if (font.equals("Times")) {
      font = "Times New Roman";
    }

    int style = Font.PLAIN;
    if (bold) {
      style += Font.BOLD;
    }
    if (italic) {
      style += Font.ITALIC;
    }
    currentFont = new Font(font, style, pixelSize);
  }


  /**
   * Flush buffers and update display.
   *
   * Only actually reacts if there are no more messages in the stack, to prevent
   * the canvas from flickering.
   */
  public void update() {
    try {
      repaint();
    } catch (NullPointerException e) {
    } catch (IllegalPathStateException e) {
    }
  }

  /** Adds a checkbox entry to the menubar, c.f. SVMenubar.add(...) */
  public void addMenuBarItem(String parent, String name, int id, boolean checked) {
    svMenuBar.add(parent, name, id, checked);
  }

  /** Adds a submenu to the menubar, c.f. SVMenubar.add(...) */
  public void addMenuBarItem(String parent, String name) {
    addMenuBarItem(parent, name, -1);
  }

  /** Adds a new entry to the menubar, c.f. SVMenubar.add(...) */
  public void addMenuBarItem(String parent, String name, int id) {
    if (svMenuBar == null) {
      svMenuBar = new SVMenuBar(this);

    }
    svMenuBar.add(parent, name, id);
  }

  /** Add a message to the message box. */
  public void addMessage(String message) {
    if (ta != null) {
      ta.append(message + "\n");
    } else {
      System.out.println(message + "\n");
    }
  }

  /**
   * This method converts a string which might contain hexadecimal values to a
   * string which contains the respective unicode counterparts.
   *
   * For example, Hall0x0094chen returns Hallöchen.
   *
   * @param input The original string, containing 0x values
   * @return The converted string which has the replaced unicode symbols
   */
  private static String convertIntegerStringToUnicodeString(String input) {
    StringBuffer sb = new StringBuffer(input);
    Pattern numbers = Pattern.compile("0x[0-9a-fA-F]{4}");
    Matcher matcher = numbers.matcher(sb);

    while (matcher.find()) {
      // Find the next match which resembles a hexadecimal value and convert it
      // to
      // its char value
      char a = (char) (Integer.decode(matcher.group()).intValue());

      // Replace the original with the new character
      sb.replace(matcher.start(), matcher.end(), String.valueOf(a));

      // Start again, since our positions have switched
      matcher.reset();
    }
    return sb.toString();
  }

  /**
   * Show a modal input dialog. The answer by the dialog is then send to the
   * client, together with the associated menu id, as SVET_POPUP
   *
   * @param msg The text that is displayed in the dialog.
   * @param def The default value of the dialog.
   * @param id The associated commandId
   * @param evtype The event this is associated with (usually SVET_MENU or SVET_POPUP)
   */
  public void showInputDialog(String msg, String def, int id, SVEventType evtype) {
    svEventHandler.timer.stop();
    String tmp =
        (String) JOptionPane.showInputDialog(this, msg, "",
            JOptionPane.QUESTION_MESSAGE, null, null, def);

    if (tmp != null) {
      tmp = convertIntegerStringToUnicodeString(tmp);
      SVEvent res = new SVEvent(evtype, this, id, tmp);
      ScrollView.addMessage(res);
    }
    svEventHandler.timer.restart();
  }


  /**
   * Shows a modal input dialog to the user. The return value is automatically
   * sent to the client as SVET_INPUT event (with command id -1).
   *
   * @param msg The text of the dialog.
   */
  public void showInputDialog(String msg) {
    showInputDialog(msg, null, -1, SVEventType.SVET_POPUP);
  }

  /**
   * Shows a dialog presenting "Yes" and "No" as answers and returns either a
   * "y" or "n" to the client.
   *
   * @param msg The text that is displayed in the dialog.
   */
  public void showYesNoDialog(String msg) {
    // res returns 0 on yes, 1 on no. Seems to be a bit counterintuitive
    int res =
        JOptionPane.showOptionDialog(this, msg, "", JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, null, null);
    SVEvent e = null;

    if (res == 0) {
      e = new SVEvent(SVEventType.SVET_INPUT, this, 0, 0, 0, 0, "y");
    } else if (res == 1) {
      e = new SVEvent(SVEventType.SVET_INPUT, this, 0, 0, 0, 0, "n");
    }
    ScrollView.addMessage(e);
  }

  /** Adds a submenu to the popup menu, c.f. SVPopupMenu.add(...) */
  public void addPopupMenuItem(String parent, String name) {
    if (svPuMenu == null) {
      svPuMenu = new SVPopupMenu(this);
    }
    svPuMenu.add(parent, name, -1);
  }

  /** Adds a new menu entry to the popup menu, c.f. SVPopupMenu.add(...) */
  public void addPopupMenuItem(String parent, String name, int cmdEvent,
      String value, String desc) {
    if (svPuMenu == null) {
      svPuMenu = new SVPopupMenu(this);
    }
    svPuMenu.add(parent, name, cmdEvent, value, desc);
  }

  /** Destroys a window. */
  public void destroy() {
    ScrollView.addMessage(new SVEvent(SVEventType.SVET_DESTROY, this, 0,
        "SVET_DESTROY"));
    setVisible(false);
    // dispose();
  }

  /**
   * Open an image from a given file location and store it in memory. Pro:
   * Faster than createImage. Con: Works only on the local file system.
   *
   * @param filename The path to the image.
   */
  public void openImage(String filename) {
    SVImageHandler.openImage(filename);
  }

}
