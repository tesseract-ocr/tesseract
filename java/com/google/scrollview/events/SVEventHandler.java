// Copyright 2007 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); You may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by
// applicable law or agreed to in writing, software distributed under the
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

package com.google.scrollview.events;

import com.google.scrollview.ScrollView;
import com.google.scrollview.events.SVEvent;
import com.google.scrollview.events.SVEventType;
import com.google.scrollview.ui.SVWindow;

import org.piccolo2d.PCamera;
import org.piccolo2d.PNode;
import org.piccolo2d.event.PBasicInputEventHandler;
import org.piccolo2d.event.PInputEvent;
import org.piccolo2d.nodes.PPath;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.Window;

import javax.swing.Timer;

/**
 * The ScrollViewEventHandler takes care of any events which might happen on the
 * canvas and converts them to an according SVEvent, which is (using the
 * processEvent method) then added to a message queue. All events from the
 * message queue get sent gradually.
 *
 * @author wanke@google.com
 */
public class SVEventHandler extends PBasicInputEventHandler implements
    ActionListener, KeyListener, WindowListener {

  /** Necessary to wait for a defined period of time (for SVET_HOVER). */
  public Timer timer;

  /** The window which the event corresponds to. */
  private SVWindow svWindow;

  /** These are used to determine a selection size (for SVET_SELECTION). */
  private int lastX = 0;
  private int lastY = 0;

  /**
   * These are used in case we want to transmit our position, but do not get it
   * because it was no MouseEvent, in particular SVET_HOVER and SVET_INPUT.
   */
  private int lastXMove = 0;
  private int lastYMove = 0;

  /** For Drawing a rubber-band rectangle for selection. */
  private int startX = 0;
  private int startY = 0;
  private float rubberBandTransparency = 0.5f;
  private PNode selection = null;

  /** The string entered since the last enter. Since the client
   *  end eats all newlines, we can't use the newline
   *  character, so use ! for now, as it cannot be entered
   *  directly anyway and therefore can never show up for real. */
  private String keyStr = "!";

  /** Setup the timer. */
  public SVEventHandler(SVWindow wdw) {
    timer = new Timer(1000, this);
    svWindow = wdw;
  }

  /**
   * Store the newest x,y values, add the message to the queue and restart the
   * timer.
   */
  private void processEvent(SVEvent e) {
    lastXMove = e.x;
    lastYMove = e.y;
    ScrollView.addMessage(e);
    timer.restart();
  }

  /** Show the associated popup menu at (x,y) (relative position of the window). */
  private void showPopup(PInputEvent e) {
    double x = e.getCanvasPosition().getX();
    double y = e.getCanvasPosition().getY();

    if (svWindow.svPuMenu != null) {
      svWindow.svPuMenu.show(svWindow, (int) x, (int) y);
    }
  }


  /** The mouse is clicked - create an SVET_CLICK event. */
  @Override
  public void mouseClicked(PInputEvent e) {
    if (e.isPopupTrigger()) {
      showPopup(e);
    } else {
      processEvent(new SVEvent(SVEventType.SVET_CLICK, svWindow, (int) e
          .getPosition().getX(), (int) e.getPosition().getY(), 0, 0, null));
    }
  }

  /**
   * The mouse key is pressed (and keeps getting pressed).
   * Depending on the OS, show a popup menu (if the button pressed is associated
   * with popup menus, like the RMB under windows&linux) or otherwise save the
   * position (in case it is a selection).
   */
  @Override
  public void mousePressed(PInputEvent e) {
    if (e.isPopupTrigger()) {
      showPopup(e);
    } else {
      lastX = (int) e.getPosition().getX();
      lastY = (int) e.getPosition().getY();
      timer.restart();
    }
  }

  /** The mouse is getting dragged - create an SVET_MOUSE event. */
  @Override
  public void mouseDragged(PInputEvent e) {
    processEvent(new SVEvent(SVEventType.SVET_MOUSE, svWindow, (int) e
        .getPosition().getX(), (int) e.getPosition().getY(), (int) e
        .getPosition().getX()
        - lastX, (int) e.getPosition().getY() - lastY, null));

    // Paint a selection rectangle.
    if (selection == null) {
      startX = (int) e.getPosition().getX();
      startY = (int) e.getPosition().getY();
      selection = PPath.createRectangle(startX, startY, 1, 1);
      selection.setTransparency(rubberBandTransparency);
      svWindow.canvas.getLayer().addChild(selection);
    } else {
      int right = Math.max(startX, (int) e.getPosition().getX());
      int left = Math.min(startX, (int) e.getPosition().getX());
      int bottom = Math.max(startY, (int) e.getPosition().getY());
      int top = Math.min(startY, (int) e.getPosition().getY());
      svWindow.canvas.getLayer().removeChild(selection);
      selection = PPath.createRectangle(left, top, right - left, bottom - top);
      selection.setPaint(Color.YELLOW);
      selection.setTransparency(rubberBandTransparency);
      svWindow.canvas.getLayer().addChild(selection);
    }
  }

  /**
   * The mouse was released.
   * Depending on the OS, show a popup menu (if the button pressed is associated
   * with popup menus, like the RMB under windows&linux) or otherwise create an
   * SVET_SELECTION event.
   */
  @Override
  public void mouseReleased(PInputEvent e) {
    if (e.isPopupTrigger()) {
      showPopup(e);
    } else {
      processEvent(new SVEvent(SVEventType.SVET_SELECTION, svWindow, (int) e
          .getPosition().getX(), (int) e.getPosition().getY(), (int) e
          .getPosition().getX()
          - lastX, (int) e.getPosition().getY() - lastY, null));
    }
    if (selection != null) {
      svWindow.canvas.getLayer().removeChild(selection);
      selection = null;
    }
  }

  /**
   * The mouse wheel is used to zoom in and out of the viewport and center on
   * the (x,y) position the mouse is currently on.
   */
  @Override
  public void mouseWheelRotated(PInputEvent e) {
    PCamera lc = svWindow.canvas.getCamera();
    double sf = SVWindow.SCALING_FACTOR;

    if (e.getWheelRotation() < 0) {
      sf = 1 / sf;
    }
    lc.scaleViewAboutPoint(lc.getScale() / sf, e.getPosition().getX(), e
        .getPosition().getY());
  }

  /**
   * The mouse was moved - create an SVET_MOTION event. NOTE: This obviously
   * creates a lot of traffic and, depending on the type of application, could
   * quite possibly be disabled.
   */
  @Override
  public void mouseMoved(PInputEvent e) {
    processEvent(new SVEvent(SVEventType.SVET_MOTION, svWindow, (int) e
        .getPosition().getX(), (int) e.getPosition().getY(), 0, 0, null));
  }

  /**
   * The mouse entered the window.
   * Start the timer, which will then emit SVET_HOVER events every X ms. */
  @Override
  public void mouseEntered(PInputEvent e) {
    timer.restart();
  }

  /**
   * The mouse exited the window
   * Stop the timer, so no more SVET_HOVER events will emit. */
  @Override
  public void mouseExited(PInputEvent e) {
    timer.stop();
  }

  /**
   * The only associated object with this is the timer, so we use it to send a
   * SVET_HOVER event.
   */
  public void actionPerformed(ActionEvent e) {
    processEvent(new SVEvent(SVEventType.SVET_HOVER, svWindow, lastXMove,
        lastYMove, 0, 0, null));
  }

  /**
   * A key was pressed - create an SVET_INPUT event.
   *
   * NOTE: Might be useful to specify hotkeys.
   *
   * Implementation note: The keyListener provided by Piccolo seems to be
   * broken, so we use the AWT listener directly.
   * There are never any keyTyped events received either so we are
   * stuck with physical keys, which is very ugly.
   */
  public void keyPressed(KeyEvent e) {
    char keyCh = e.getKeyChar();
    if (keyCh == '\r' || keyCh == '\n' || keyCh == '\0' || keyCh == '?') {
      processEvent(new SVEvent(SVEventType.SVET_INPUT, svWindow, lastXMove,
                               lastYMove, 0, 0, keyStr));
      // Send newline characters as '!' as '!' can never be a keypressed
      // and the client eats all newline characters.
      keyStr = "!";
    } else {
      processEvent(new SVEvent(SVEventType.SVET_INPUT, svWindow, lastXMove,
                               lastYMove, 0, 0, String.valueOf(keyCh)));
      keyStr += keyCh;
    }
  }

  /**
   * A window is closed (by the 'x') - create an SVET_DESTROY event. If it was
   * the last open Window, also send an SVET_EXIT event (but do not exit unless
   * the client says so).
   */
  public void windowClosing(WindowEvent e) {
    processEvent(new SVEvent(SVEventType.SVET_DESTROY, svWindow, lastXMove,
        lastYMove, 0, 0, null));
    Window w = e.getWindow();
    if (w != null) {
      w.dispose();
    }
    SVWindow.nrWindows--;
    if (SVWindow.nrWindows == 0) {
      processEvent(new SVEvent(SVEventType.SVET_EXIT, svWindow, lastXMove,
          lastYMove, 0, 0, null));
    }
  }

  /** These are all events we do not care about and throw away. */
  public void keyReleased(KeyEvent e) {
  }

  public void keyTyped(KeyEvent e) {
  }

  public void windowActivated(WindowEvent e) {
  }

  public void windowClosed(WindowEvent e) {
  }

  public void windowDeactivated(WindowEvent e) {
  }

  public void windowDeiconified(WindowEvent e) {
  }

  public void windowIconified(WindowEvent e) {
  }

  public void windowOpened(WindowEvent e) {
  }
}
