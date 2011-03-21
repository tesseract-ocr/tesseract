///////////////////////////////////////////////////////////////////////
// File:        scrollview.cc
// Description: ScrollView
// Author:      Joern Wanke
// Created:     Thu Nov 29 2007
//
// (C) Copyright 2007, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////
//

#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <map>
#include <utility>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <climits>

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifndef GRAPHICS_DISABLED
// This class contains the main ScrollView-logic,
// e.g. parsing & sending messages, images etc.
#ifdef _MSC_VER
#pragma warning(disable:4786)  // Don't give stupid warnings for stl
#pragma warning(disable:4018)  // signed/unsigned warnings
#pragma warning(disable:4530)  // exception warnings
#endif

const int kSvPort = 8461;
const int kMaxMsgSize = 4096;
const int kMaxIntPairSize = 45;  // Holds %d,%d, for upto 64 bit.

#include "scrollview.h"

#include "svutil.h"

#ifdef HAVE_LIBLEPT
#include "allheaders.h"
#endif

struct SVPolyLineBuffer {
  bool empty;  // Independent indicator to allow SendMsg to call SendPolygon.
  std::vector<int> xcoords;
  std::vector<int> ycoords;
};

// A map between the window IDs and their corresponding pointers.
static std::map<int, ScrollView*> svmap;
static SVMutex* svmap_mu;
// A map of all semaphores waiting for a specific event on a specific window.
static std::map<std::pair<ScrollView*, SVEventType>,
                std::pair<SVSemaphore*, SVEvent*> > waiting_for_events;
static SVMutex* waiting_for_events_mu;

SVEvent* SVEvent::copy() {
  SVEvent* any = new SVEvent;
  any->command_id = command_id;
  any->counter = counter;
  any->parameter = new char[strlen(parameter) + 1];
  strncpy(any->parameter, parameter, strlen(parameter));
  any->parameter[strlen(parameter)] = '\0';
  any->type = type;
  any->x = x;
  any->y = y;
  any->x_size = x_size;
  any->y_size = y_size;
  any->window = window;
  return any;
}

/// This is the main loop which handles the ScrollView-logic from the server
/// to the client. It basically loops through messages, parses them to events
/// and distributes it to the waiting handlers.
/// It is run from a different thread and synchronizes via SVSync.
void* ScrollView::MessageReceiver(void* a) {
  int counter_event_id = 0;  // ongoing counter
  char* message = NULL;
  // Wait until a new message appears in the input stream_.
  do {
    message = ScrollView::GetStream()->Receive();
  } while (message == NULL);

// This is the main loop which iterates until the server is dead (strlen = -1).
// It basically parses for 3 different messagetypes and then distributes the
// events accordingly.
  while (strlen(message) != -1) {
      // The new event we create.
      SVEvent* cur = new SVEvent;
      // The ID of the corresponding window.
      int window_id;

      int ev_type;

      int n;
      // Fill the new SVEvent properly.
      sscanf(message, "%d,%d,%d,%d,%d,%d,%d,%n", &window_id, &ev_type, &cur->x,
             &cur->y, &cur->x_size, &cur->y_size, &cur->command_id, &n);
      char* p = (message + n);

      svmap_mu->Lock();
      cur->window = svmap[window_id];

      if (cur->window != NULL) {
        cur->parameter = new char[strlen(p) + 1];
        strncpy(cur->parameter, p, strlen(p) + 1);
        if (strlen(p) > 0) {  // remove the last \n
          cur->parameter[strlen(p)] = '\0';
        }
        cur->type = static_cast<SVEventType>(ev_type);
        // Correct selection coordinates so x,y is the min pt and size is +ve.
        if (cur->x_size > 0)
          cur->x -= cur->x_size;
        else
          cur->x_size = -cur->x_size;
        if (cur->y_size > 0)
          cur->y -= cur->y_size;
        else
          cur->y_size = -cur->y_size;
        // Returned y will be the bottom-left if y is reversed.
        if (cur->window->y_axis_is_reversed_)
          cur->y = cur->window->TranslateYCoordinate(cur->y + cur->y_size);
        cur->counter = counter_event_id;
        // Increase by 2 since we will also create an SVET_ANY event from cur,
        // which will have a counter_id of cur + 1 (and thus gets processed
        // after cur).
        counter_event_id += 2;

        // In case of an SVET_EXIT event, quit the whole application.
        if (ev_type == SVET_EXIT) { ScrollView::Exit(); }

        // Place two copies of it in the table for the window.
        cur->window->SetEvent(cur);

        // Check if any of the threads currently waiting want it.
        std::pair<ScrollView*, SVEventType> awaiting_list(cur->window,
                                                          cur->type);
        std::pair<ScrollView*, SVEventType> awaiting_list_any(cur->window,
                                                              SVET_ANY);
        std::pair<ScrollView*, SVEventType> awaiting_list_any_window((ScrollView*)0,
                                                              SVET_ANY);
        waiting_for_events_mu->Lock();
        if (waiting_for_events.count(awaiting_list) > 0) {
          waiting_for_events[awaiting_list].second = cur;
          waiting_for_events[awaiting_list].first->Signal();
        } else if (waiting_for_events.count(awaiting_list_any) > 0) {
          waiting_for_events[awaiting_list_any].second = cur;
          waiting_for_events[awaiting_list_any].first->Signal();
        } else if (waiting_for_events.count(awaiting_list_any_window) > 0) {
          waiting_for_events[awaiting_list_any_window].second = cur;
          waiting_for_events[awaiting_list_any_window].first->Signal();
        } else {
          // No one wanted it, so delete it.
          delete cur;
        }
        waiting_for_events_mu->Unlock();
        // Signal the corresponding semaphore twice (for both copies).
        ScrollView* sv = svmap[window_id];
        if (sv != NULL) {
          sv->Signal();
          sv->Signal();
        }
      }
      svmap_mu->Unlock();

      // Wait until a new message appears in the input stream_.
      do {
        message = ScrollView::GetStream()->Receive();
      } while (message == NULL);
    }
  return 0;
}

// Table to implement the color index values in the old system.
int table_colors[ScrollView::GREEN_YELLOW+1][4]= {
  {0, 0, 0, 0},        // NONE (transparent)
  {0, 0, 0, 255},        // BLACK.
  {255, 255, 255, 255},  // WHITE.
  {255, 0, 0, 255},      // RED.
  {255, 255, 0, 255},    // YELLOW.
  {0, 255, 0, 255},      // GREEN.
  {0, 255, 255, 255},    // CYAN.
  {0, 0, 255, 255},      // BLUE.
  {255, 0, 255, 255},    // MAGENTA.
  {0, 128, 255, 255},    // AQUAMARINE.
  {0, 0, 64, 255},       // DARK_SLATE_BLUE.
  {128, 128, 255, 255},  // LIGHT_BLUE.
  {64, 64, 255, 255},    // MEDIUM_BLUE.
  {0, 0, 32, 255},       // MIDNIGHT_BLUE.
  {0, 0, 128, 255},      // NAVY_BLUE.
  {192, 192, 255, 255},  // SKY_BLUE.
  {64, 64, 128, 255},    // SLATE_BLUE.
  {32, 32, 64, 255},     // STEEL_BLUE.
  {255, 128, 128, 255},  // CORAL.
  {128, 64, 0, 255},     // BROWN.
  {128, 128, 0, 255},    // SANDY_BROWN.
  {192, 192, 0, 255},    // GOLD.
  {192, 192, 128, 255},  // GOLDENROD.
  {0, 64, 0, 255},       // DARK_GREEN.
  {32, 64, 0, 255},      // DARK_OLIVE_GREEN.
  {64, 128, 0, 255},     // FOREST_GREEN.
  {128, 255, 0, 255},    // LIME_GREEN.
  {192, 255, 192, 255},  // PALE_GREEN.
  {192, 255, 0, 255},    // YELLOW_GREEN.
  {192, 192, 192, 255},  // LIGHT_GREY.
  {64, 64, 128, 255},    // DARK_SLATE_GREY.
  {64, 64, 64, 255},     // DIM_GREY.
  {128, 128, 128, 255},  // GREY.
  {64, 192, 0, 255},     // KHAKI.
  {255, 0, 192, 255},    // MAROON.
  {255, 128, 0, 255},    // ORANGE.
  {255, 128, 64, 255},   // ORCHID.
  {255, 192, 192, 255},  // PINK.
  {128, 0, 128, 255},    // PLUM.
  {255, 0, 64, 255},     // INDIAN_RED.
  {255, 64, 0, 255},     // ORANGE_RED.
  {255, 0, 192, 255},    // VIOLET_RED.
  {255, 192, 128, 255},  // SALMON.
  {128, 128, 0, 255},    // TAN.
  {0, 255, 255, 255},    // TURQUOISE.
  {0, 128, 128, 255},    // DARK_TURQUOISE.
  {192, 0, 255, 255},    // VIOLET.
  {128, 128, 0, 255},    // WHEAT.
  {128, 255, 0, 255}     // GREEN_YELLOW
};


/*******************************************************************************
* Scrollview implementation.
*******************************************************************************/

SVNetwork* ScrollView::stream_ = NULL;
int ScrollView::nr_created_windows_ = 0;
int ScrollView::image_index_ = 0;

/// Calls Initialize with all arguments given.
ScrollView::ScrollView(const char* name, int x_pos, int y_pos, int x_size,
                       int y_size, int x_canvas_size, int y_canvas_size,
                       bool y_axis_reversed, const char* server_name) {
  Initialize(name, x_pos, y_pos, x_size, y_size, x_canvas_size, y_canvas_size,
             y_axis_reversed, server_name);}

/// Calls Initialize with default argument for server_name_.
ScrollView::ScrollView(const char* name, int x_pos, int y_pos, int x_size,
                       int y_size, int x_canvas_size, int y_canvas_size,
                       bool y_axis_reversed) {
  Initialize(name, x_pos, y_pos, x_size, y_size, x_canvas_size, y_canvas_size,
             y_axis_reversed, "localhost");
}

/// Calls Initialize with default argument for server_name_ & y_axis_reversed.
ScrollView::ScrollView(const char* name, int x_pos, int y_pos, int x_size,
                       int y_size, int x_canvas_size, int y_canvas_size) {
  Initialize(name, x_pos, y_pos, x_size, y_size, x_canvas_size, y_canvas_size,
             false, "localhost");
}

/// Sets up a ScrollView window, depending on the constructor variables.
void ScrollView::Initialize(const char* name, int x_pos, int y_pos, int x_size,
                            int y_size, int x_canvas_size, int y_canvas_size,
                            bool y_axis_reversed, const char* server_name) {
  // If this is the first ScrollView Window which gets created, there is no
  // network connection yet and we have to set it up in a different thread.
  if (stream_ == NULL) {
    nr_created_windows_ = 0;
    stream_ = new SVNetwork(server_name, kSvPort);
    waiting_for_events_mu = new SVMutex();
    svmap_mu = new SVMutex();
    SendRawMessage(
        "svmain = luajava.bindClass('com.google.scrollview.ScrollView')\n");
    SVSync::StartThread(MessageReceiver, NULL);
  }

  // Set up the variables on the clientside.
  nr_created_windows_++;
  event_handler_ = NULL;
  event_handler_ended_ = false;
  y_axis_is_reversed_ = y_axis_reversed;
  y_size_ = y_canvas_size;
  window_name_ = name;
  window_id_ = nr_created_windows_;
  // Set up polygon buffering.
  points_ = new SVPolyLineBuffer;
  points_->empty = true;

  svmap_mu->Lock();
  svmap[window_id_] = this;
  svmap_mu->Unlock();

  for (int i = 0; i < SVET_COUNT; i++) {
    event_table_[i] = NULL;
  }

  mutex_ = new SVMutex();
  semaphore_ = new SVSemaphore();

  // Set up an actual Window on the client side.
  char message[kMaxMsgSize];
  snprintf(message, sizeof(message),
           "w%u = luajava.newInstance('com.google.scrollview.ui"
           ".SVWindow','%s',%u,%u,%u,%u,%u,%u,%u)\n",
           window_id_, window_name_, window_id_,
           x_pos, y_pos, x_size, y_size, x_canvas_size, y_canvas_size);
  SendRawMessage(message);

  SVSync::StartThread(StartEventHandler, this);
}

/// Sits and waits for events on this window.
void* ScrollView::StartEventHandler(void* a) {
  ScrollView* sv = reinterpret_cast<ScrollView*>(a);
  SVEvent* new_event;

  do {
    stream_->Flush();
    sv->semaphore_->Wait();
    new_event = NULL;
    int serial = -1;
    int k = -1;
    sv->mutex_->Lock();
    // Check every table entry if he is is valid and not already processed.

    for (int i = 0; i < SVET_COUNT; i++) {
      if (sv->event_table_[i] != NULL &&
          (serial < 0 || sv->event_table_[i]->counter < serial)) {
        new_event = sv->event_table_[i];
        serial = sv->event_table_[i]->counter;
        k = i;
      }
    }
    // If we didnt find anything we had an old alarm and just sleep again.
    if (new_event != NULL) {
      sv->event_table_[k] = NULL;
      sv->mutex_->Unlock();
      if (sv->event_handler_ != NULL) { sv->event_handler_->Notify(new_event); }
      if (new_event->type == SVET_DESTROY) {
        // Signal the destructor that it is safe to terminate.
        sv->event_handler_ended_ = true;
        sv = NULL;
      }
      delete new_event;  // Delete the pointer after it has been processed.
    } else { sv->mutex_->Unlock(); }
  // The thread should run as long as its associated window is alive.
  } while (sv != NULL);
  return 0;
}

ScrollView::~ScrollView() {
  svmap_mu->Lock();
  if (svmap[window_id_] != NULL) {
    svmap_mu->Unlock();
    // So the event handling thread can quit.
    SendMsg("destroy()");

    SVEvent* sve = AwaitEvent(SVET_DESTROY);
    delete sve;
    svmap_mu->Lock();
    svmap[window_id_] = NULL;
    svmap_mu->Unlock();
    // The event handler thread for this window *must* receive the
    // destroy event and set its pointer to this to NULL before we allow
    // the destructor to exit.
    while (!event_handler_ended_)
      Update();
  } else {
    svmap_mu->Unlock();
  }
  delete mutex_;
  delete semaphore_;
  delete points_;
}

/// Send a message to the server, attaching the window id.
void ScrollView::SendMsg(const char* format, ...) {
  if (!points_->empty)
    SendPolygon();
  va_list args;
  char message[kMaxMsgSize];

  va_start(args, format);  // variable list
  vsnprintf(message, kMaxMsgSize, format, args);
  va_end(args);

  char form[kMaxMsgSize];
  snprintf(form, kMaxMsgSize, "w%u:%s\n", window_id_, message);

  stream_->Send(form);
}

/// Send a message to the server without a
/// window id. Used for global events like exit().
void ScrollView::SendRawMessage(const char* msg) {
  stream_->Send(msg);
}

/// Add an Event Listener to this ScrollView Window
void ScrollView::AddEventHandler(SVEventHandler* listener) {
  event_handler_ = listener;
}

void ScrollView::Signal() {
  semaphore_->Signal();
}

void ScrollView::SetEvent(SVEvent* svevent) {
// Copy event
  SVEvent* any = svevent->copy();
  SVEvent* specific = svevent->copy();
  any->counter = specific->counter + 1;

// Place both events into the queue.
  mutex_->Lock();
  // Delete the old objects..
  if (event_table_[specific->type] != NULL) {
    delete event_table_[specific->type]; }
  if (event_table_[SVET_ANY] != NULL) {
    delete event_table_[SVET_ANY]; }
  // ...and put the new ones in the table.
  event_table_[specific->type] = specific;
  event_table_[SVET_ANY] = any;
  mutex_->Unlock();
}


/// Block until an event of the given type is received.
/// Note: The calling function is responsible for deleting the returned
/// SVEvent afterwards!
SVEvent* ScrollView::AwaitEvent(SVEventType type) {
  // Initialize the waiting semaphore.
  SVSemaphore* sem = new SVSemaphore();
  std::pair<ScrollView*, SVEventType> ea(this, type);
  waiting_for_events_mu->Lock();
  waiting_for_events[ea] = std::pair<SVSemaphore*, SVEvent*> (sem, (SVEvent*)0);
  waiting_for_events_mu->Unlock();
  // Wait on it, but first flush.
  stream_->Flush();
  sem->Wait();
  // Process the event we got woken up for (its in waiting_for_events pair).
  waiting_for_events_mu->Lock();
  SVEvent* ret = waiting_for_events[ea].second;
  waiting_for_events.erase(ea);
  delete sem;
  waiting_for_events_mu->Unlock();
  return ret;
}

// Block until any event on any window is received.
// No event is returned here!
SVEvent* ScrollView::AwaitEventAnyWindow() {
  // Initialize the waiting semaphore.
  SVSemaphore* sem = new SVSemaphore();
  std::pair<ScrollView*, SVEventType> ea((ScrollView*)0, SVET_ANY);
  waiting_for_events_mu->Lock();
  waiting_for_events[ea] = std::pair<SVSemaphore*, SVEvent*> (sem, (SVEvent*)0);
  waiting_for_events_mu->Unlock();
  // Wait on it.
  stream_->Flush();
  sem->Wait();
  // Process the event we got woken up for (its in waiting_for_events pair).
  waiting_for_events_mu->Lock();
  SVEvent* ret = waiting_for_events[ea].second;
  waiting_for_events.erase(ea);
  waiting_for_events_mu->Unlock();
  return ret;
}

// Send the current buffered polygon (if any) and clear it.
void ScrollView::SendPolygon() {
  if (!points_->empty) {
    points_->empty = true;  // Allows us to use SendMsg.
    int length = points_->xcoords.size();
    // length == 1 corresponds to 2 SetCursors in a row and only the
    // last setCursor has any effect.
    if (length == 2) {
      // An isolated line!
      SendMsg("drawLine(%d,%d,%d,%d)",
              points_->xcoords[0], points_->ycoords[0],
              points_->xcoords[1], points_->ycoords[1]);
    } else if (length > 2) {
      // A polyline.
      SendMsg("createPolyline(%d)", length);
      char coordpair[kMaxIntPairSize];
      std::string decimal_coords;
      for (int i = 0; i < length; ++i) {
        snprintf(coordpair, kMaxIntPairSize, "%d,%d,",
                 points_->xcoords[i], points_->ycoords[i]);
        decimal_coords += coordpair;
      }
      decimal_coords += '\n';
      SendRawMessage(decimal_coords.c_str());
      SendMsg("drawPolyline()");
    }
    points_->xcoords.clear();
    points_->ycoords.clear();
  }
}


/*******************************************************************************
* LUA "API" functions.
*******************************************************************************/

// Sets the position from which to draw to (x,y).
void ScrollView::SetCursor(int x, int y) {
  SendPolygon();
  DrawTo(x, y);
}

// Draws from the current position to (x,y) and sets the new position to it.
void ScrollView::DrawTo(int x, int y) {
  points_->xcoords.push_back(x);
  points_->ycoords.push_back(TranslateYCoordinate(y));
  points_->empty = false;
}

// Draw a line using the current pen color.
void ScrollView::Line(int x1, int y1, int x2, int y2) {
  if (!points_->xcoords.empty() && x1 == points_->xcoords.back() &&
      TranslateYCoordinate(y1) == points_->ycoords.back()) {
    // We are already at x1, y1, so just draw to x2, y2.
    DrawTo(x2, y2);
  } else if (!points_->xcoords.empty() && x2 == points_->xcoords.back() &&
      TranslateYCoordinate(y2) == points_->ycoords.back()) {
    // We are already at x2, y2, so just draw to x1, y1.
    DrawTo(x1, y1);
  } else {
    // This is a new line.
    SetCursor(x1, y1);
    DrawTo(x2, y2);
  }
}

// Set the visibility of the window.
void ScrollView::SetVisible(bool visible) {
  if (visible) { SendMsg("setVisible(true)");
  } else { SendMsg("setVisible(false)"); }
}

// Set the alwaysOnTop flag.
void ScrollView::AlwaysOnTop(bool b) {
  if (b) { SendMsg("setAlwaysOnTop(true)");
  } else { SendMsg("setAlwaysOnTop(false)"); }
}

// Adds a message entry to the message box.
void ScrollView::AddMessage(const char* format, ...) {
  va_list args;
  char message[kMaxMsgSize];
  char form[kMaxMsgSize];

  va_start(args, format);  // variable list
  vsnprintf(message, kMaxMsgSize, format, args);
  va_end(args);

  snprintf(form, kMaxMsgSize, "w%u:%s", window_id_, message);

  char* esc = AddEscapeChars(form);
  SendMsg("addMessage(\"%s\")", esc);
  delete[] esc;
}

// Set a messagebox.
void ScrollView::AddMessageBox() {
  SendMsg("addMessageBox()");
}

// Exit the client completely (and notify the server of it).
void ScrollView::Exit() {
  SendRawMessage("svmain:exit()");
  exit(0);
}

// Clear the canvas.
void ScrollView::Clear() {
  SendMsg("clear()");
}

// Set the stroke width.
void ScrollView::Stroke(float width) {
  SendMsg("setStrokeWidth(%f)", width);
}

// Draw a rectangle using the current pen color.
// The rectangle is filled with the current brush color.
void ScrollView::Rectangle(int x1, int y1, int x2, int y2) {
  if (x1 == x2 && y1 == y2)
    return;  // Scrollviewer locks up.
  SendMsg("drawRectangle(%d,%d,%d,%d)",
    x1, TranslateYCoordinate(y1), x2, TranslateYCoordinate(y2));
}

// Draw an ellipse using the current pen color.
// The ellipse is filled with the current brush color.
void ScrollView::Ellipse(int x1, int y1, int width, int height) {
  SendMsg("drawEllipse(%d,%d,%u,%u)",
    x1, TranslateYCoordinate(y1), width, height);
}

// Set the pen color to the given RGB values.
void ScrollView::Pen(int red, int green, int blue) {
  SendMsg("pen(%d,%d,%d)", red, green, blue);
}

// Set the pen color to the given RGB values.
void ScrollView::Pen(int red, int green, int blue, int alpha) {
  SendMsg("pen(%d,%d,%d,%d)", red, green, blue, alpha);
}

// Set the brush color to the given RGB values.
void ScrollView::Brush(int red, int green, int blue) {
  SendMsg("brush(%d,%d,%d)", red, green, blue);
}

// Set the brush color to the given RGB values.
void ScrollView::Brush(int red, int green, int blue, int alpha) {
  SendMsg("brush(%d,%d,%d,%d)", red, green, blue, alpha);
}

// Set the attributes for future Text(..) calls.
void ScrollView::TextAttributes(const char* font, int pixel_size,
                                bool bold, bool italic, bool underlined) {
  const char* b;
  const char* i;
  const char* u;

  if (bold) { b = "true";
  } else { b = "false"; }
  if (italic) { i = "true";
  } else { i = "false"; }
  if (underlined) { u = "true";
  } else { u = "false"; }
  SendMsg("textAttributes('%s',%u,%s,%s,%s)", font, pixel_size,
    b, i, u);
}

// Draw text at the given coordinates.
void ScrollView::Text(int x, int y, const char* mystring) {
  SendMsg("drawText(%d,%d,'%s')", x, TranslateYCoordinate(y), mystring);
}

// Open and draw an image given a name at (x,y).
void ScrollView::Image(const char* image, int x_pos, int y_pos) {
  SendMsg("openImage('%s')", image);
  SendMsg("drawImage('%s',%d,%d)",
                image, x_pos, TranslateYCoordinate(y_pos));
}

// Add new checkboxmenuentry to menubar.
void ScrollView::MenuItem(const char* parent, const char* name,
                          int cmdEvent, bool flag) {
  if (parent == NULL) { parent = ""; }
  if (flag) { SendMsg("addMenuBarItem('%s','%s',%d,true)",
                      parent, name, cmdEvent);
  } else { SendMsg("addMenuBarItem('%s','%s',%d,false)",
                   parent, name, cmdEvent); }
}

// Add new menuentry to menubar.
void ScrollView::MenuItem(const char* parent, const char* name, int cmdEvent) {
  if (parent == NULL) { parent = ""; }
  SendMsg("addMenuBarItem('%s','%s',%d)", parent, name, cmdEvent);
}

// Add new submenu to menubar.
void ScrollView::MenuItem(const char* parent, const char* name) {
  if (parent == NULL) { parent = ""; }
  SendMsg("addMenuBarItem('%s','%s')", parent, name);
}

// Add new submenu to popupmenu.
void ScrollView::PopupItem(const char* parent, const char* name) {
  if (parent == NULL) { parent = ""; }
  SendMsg("addPopupMenuItem('%s','%s')", parent, name);
}

// Add new submenuentry to popupmenu.
void ScrollView::PopupItem(const char* parent, const char* name,
                           int cmdEvent, const char* value, const char* desc) {
  if (parent == NULL) { parent = ""; }
  char* esc = AddEscapeChars(value);
  char* esc2 = AddEscapeChars(desc);
  SendMsg("addPopupMenuItem('%s','%s',%d,'%s','%s')", parent, name,
          cmdEvent, esc, esc2);
  delete[] esc;
  delete[] esc2;
}

// Send an update message for a single window.
void ScrollView::UpdateWindow() {
  SendMsg("update()");
}

// Note: this is an update to all windows
void ScrollView::Update() {
  svmap_mu->Lock();
  for (std::map<int, ScrollView*>::iterator iter = svmap.begin();
      iter != svmap.end(); ++iter) {
    if (iter->second != NULL)
      iter->second->UpdateWindow();
  }
  svmap_mu->Unlock();
}

// Set the pen color, using an enum value (e.g. ScrollView::ORANGE)
void ScrollView::Pen(Color color) {
  Pen(table_colors[color][0], table_colors[color][1],
      table_colors[color][2], table_colors[color][3]);
}

// Set the brush color, using an enum value (e.g. ScrollView::ORANGE)
void ScrollView::Brush(Color color) {
  Brush(table_colors[color][0],
        table_colors[color][1],
        table_colors[color][2],
        table_colors[color][3]);
}

// Shows a modal Input Dialog which can return any kind of String
char* ScrollView::ShowInputDialog(const char* msg) {
  SendMsg("showInputDialog(\"%s\")", msg);
  SVEvent* ev;
  // wait till an input event (all others are thrown away)
  ev = AwaitEvent(SVET_INPUT);
  char* p = new char[strlen(ev->parameter) + 1];
  strncpy(p, ev->parameter, strlen(ev->parameter));
  p[strlen(ev->parameter)] = '\0';
  delete ev;
  return p;
}

// Shows a modal Yes/No Dialog which will return 'y' or 'n'
int ScrollView::ShowYesNoDialog(const char* msg) {
  SendMsg("showYesNoDialog(\"%s\")", msg);
  SVEvent* ev;
  // Wait till an input event (all others are thrown away)
  ev = AwaitEvent(SVET_INPUT);
  int a = ev->parameter[0];
  delete ev;
  return a;
}

// Zoom the window to the rectangle given upper left corner and
// lower right corner.
void ScrollView::ZoomToRectangle(int x1, int y1, int x2, int y2) {
  y1 = TranslateYCoordinate(y1);
  y2 = TranslateYCoordinate(y2);
  SendMsg("zoomRectangle(%d,%d,%d,%d)",
          MIN(x1, x2), MIN(y1, y2), MAX(x1, x2), MAX(y1, y2));
}

#ifdef HAVE_LIBLEPT
// Send an image of type PIX.
void ScrollView::Image(PIX* image, int x_pos, int y_pos) {
  int width = image->w;
  int height = image->h;
  l_uint32 bpp = image->d;
  ++image_index_;
  // PIX* do not have a unique identifier/name associated, so name them "lept".
  SendMsg("createImage('lept%d',%d,%d,%d)", image_index_, width, height, bpp);

  if (bpp == 32) {
    Transfer32bppImage(image);
  } else if (bpp == 8) {
    TransferGrayImage(image);
  } else if (bpp == 1) {
    TransferBinaryImage(image);
  }
  // PIX* do not have a unique identifier/name associated, so name them "lept".
  SendMsg("drawImage('lept%d',%d,%d)", image_index_, x_pos, y_pos);
}

// Sends each pixel as hex value like html, e.g. #00FF00 for green.
void ScrollView::Transfer32bppImage(PIX* image) {
  int ppL = pixGetWidth(image);
  int h = pixGetHeight(image);
  int wpl = pixGetWpl(image);
  int transfer_size= ppL * 7 + 2;
  char* pixel_data = new char[transfer_size];
  for (int y = 0; y < h; ++y) {
    l_uint32* data = pixGetData(image) + y*wpl;
    for (int x = 0; x < ppL; ++x, ++data) {
      snprintf(&pixel_data[x*7], 7, "#%.2x%.2x%.2x",
               GET_DATA_BYTE(data, COLOR_RED),
               GET_DATA_BYTE(data, COLOR_GREEN),
               GET_DATA_BYTE(data, COLOR_BLUE));
    }
    pixel_data[transfer_size - 2] = '\n';
    pixel_data[transfer_size - 1] = '\0';
    SendRawMessage(pixel_data);
  }
  delete[] pixel_data;
}

// Sends for each pixel either '1' or '0'.
void ScrollView::TransferGrayImage(PIX* image) {
  char* pixel_data = new char[image->w * 2 + 2];
  for (int y = 0; y < image->h; y++) {
    l_uint32* data = pixGetData(image) + y * pixGetWpl(image);
    for (int x = 0; x < image->w; x++) {
      snprintf(&pixel_data[x*2], 2, "%.2x", (GET_DATA_BYTE(data, x)));
      pixel_data[image->w * 2] = '\n';
      pixel_data[image->w * 2 + 1] = '\0';
      SendRawMessage(pixel_data);
    }
  }
  delete [] pixel_data;
}

// Sends for each pixel either '1' or '0'.
void ScrollView::TransferBinaryImage(PIX* image) {
  char* pixel_data = new char[image->w + 2];
  for (int y = 0; y < image->h; y++) {
    l_uint32* data = pixGetData(image) + y * pixGetWpl(image);
    for (int x = 0; x < image->w; x++) {
      if (GET_DATA_BIT(data, x))
        pixel_data[x] = '1';
      else
        pixel_data[x] = '0';
    }
    pixel_data[image->w] = '\n';
    pixel_data[image->w + 1] = '\0';
    SendRawMessage(pixel_data);
  }
  delete [] pixel_data;
}
#endif

// Escapes the ' character with a \, so it can be processed by LUA.
// Note: The caller will have to make sure he deletes the newly allocated item.
char* ScrollView::AddEscapeChars(const char* input) {
  const char* nextptr = strchr(input, '\'');
  const char* lastptr = input;
  char* message = new char[kMaxMsgSize];
  int pos = 0;
  while (nextptr != NULL) {
    strncpy(message+pos, lastptr, nextptr-lastptr);
    pos += nextptr - lastptr;
    message[pos] = '\\';
    pos += 1;
    lastptr = nextptr;
    nextptr = strchr(nextptr+1, '\'');
  }
  strncpy(message+pos, lastptr, strlen(lastptr));
  message[pos+strlen(lastptr)] = '\0';
  return message;
}

// Inverse the Y axis if the coordinates are actually inversed.
int ScrollView::TranslateYCoordinate(int y) {
  if (!y_axis_is_reversed_) { return y;
  } else { return y_size_ - y; }
}

#endif  // GRAPHICS_DISABLED
