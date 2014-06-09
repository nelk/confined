#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <gtkmm.h>
#include <gtkglmm.h>

// The "main" OpenGL widget
class Viewer : public Gtk::GL::DrawingArea {
public:
  enum Mode {
    POSITION, JOINTS, NUM_MODES
  };

  enum ResetType {
    RESET_POSITION, RESET_ORIENTATION, RESET_JOINTS, RESET_ALL, NUM_RESET_TYPES
  };

  Viewer();
  virtual ~Viewer();

  // A useful function that forces this widget to rerender. If you
  // want to render a new frame, do not call on_expose_event
  // directly. Instead call this, which will cause an on_expose_event
  // call when the time is right.
  void invalidate();

  void set_mode(Mode mode);
  void reset(ResetType r);

protected:
  // Called when GL is first initialized
  virtual void on_realize();
  // Called when our window needs to be redrawn
  virtual bool on_expose_event(GdkEventExpose* event);
  // Called when the window is resized
  virtual bool on_configure_event(GdkEventConfigure* event);
  // Called when a mouse button is pressed
  virtual bool on_button_press_event(GdkEventButton* event);
  // Called when a mouse button is released
  virtual bool on_button_release_event(GdkEventButton* event);
  // Called when the mouse moves
  virtual bool on_motion_notify_event(GdkEventMotion* event);

  // Draw a circle for the trackball, with OpenGL commands.
  // Assumes the context for the viewer is active.
  void draw_trackball_circle();

private:
  Mode mode;
};

#endif
