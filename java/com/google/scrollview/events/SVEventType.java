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

/**
 * These are the defined events which can happen in ScrollView and be
 * transferred to the client. They are same events as on the client side part of
 * ScrollView (defined in ScrollView.h).
 *
 * @author wanke@google.com
 */
public enum SVEventType {
  SVET_DESTROY, // Window has been destroyed by user.
  SVET_EXIT, // User has destroyed the last window by clicking on the 'X'
  SVET_CLICK, // Any button pressed that is not a popup trigger.
  SVET_SELECTION, // Left button selection.
  SVET_INPUT, // Any kind of input
  SVET_MOUSE, // The mouse has moved with a button pressed.
  SVET_MOTION, // The mouse has moved with no button pressed.
  SVET_HOVER, // The mouse has stayed still for a second.
  SVET_POPUP, // A command selected through a popup menu
  SVET_MENU; // A command selected through the menubar
}
