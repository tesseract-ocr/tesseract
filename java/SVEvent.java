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

import com.google.scrollview.ui.SVWindow;

/**
 * The SVEvent is a structure which holds the actual values of a message to be
 * transmitted. It corresponds to the client structure defined in scrollview.h
 * 
 * @author wanke@google.com
 */
public class SVEvent {
  SVEventType type; // What kind of event.
  SVWindow window; // Window event relates to.
  int x; // Coords of click or selection.
  int y;
  int xSize; // Size of selection.
  int ySize;
  int commandId;
  String parameter; // Any string that might have been passed as argument.

  /**
   * A "normal" SVEvent.
   * 
   * @param t The type of the event as specified in SVEventType (e.g.
   *        SVET_CLICK)
   * @param w The window the event corresponds to
   * @param x1 X position of the mouse at the time of the event
   * @param y1 Y position of the mouse at the time of the event
   * @param x2 X selection size at the time of the event
   * @param y2 Y selection size at the time of the event
   * @param p A parameter associated with the event (e.g. keyboard input)
   */
  public SVEvent(SVEventType t, SVWindow w, int x1, int y1, int x2, int y2,
      String p) {
    type = t;
    window = w;
    x = x1;
    y = y1;
    xSize = x2;
    ySize = y2;
    commandId = 0;
    parameter = p;    
  }

  /**
   * An event which issues a command (like clicking on a item in the menubar).
   * 
   * @param eventtype The type of the event as specified in SVEventType
   *        (usually SVET_MENU or SVET_POPUP)
   * @param svWindow The window the event corresponds to
   * @param commandid The associated id with the command (given by the client
   *        on construction of the item)
   * @param value A parameter associated with the event (e.g. keyboard input)
   */
  public SVEvent(SVEventType eventtype, SVWindow svWindow, int commandid,
      String value) {
    type = eventtype;
    window = svWindow;

    parameter = value;
    x = 0;
    y = 0;
    xSize = 0;
    ySize = 0;
    commandId = commandid;
  }

  /**
   * This is the string representation of the message, which is what will
   * actually be transferred over the network.
   */
  @Override
  public String toString() {
    return (window.hash + "," + type.ordinal() + "," + x + "," + y + ","
        + xSize + "," + ySize + "," + commandId + "," + parameter);
  }
}
