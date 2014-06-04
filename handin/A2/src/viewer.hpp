#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <gtkmm.h>
#include <gtkglmm.h>
#include "algebra.hpp"
#include "shape.hpp"
#include "appwindow.hpp"

class AppWindow;

// The "main" OpenGL widget
class Viewer : public Gtk::GL::DrawingArea {
public:
  enum Mode {
    VIEW_ROTATE, VIEW_TRANSLATE, VIEW_PERSPECTIVE,
    MODEL_ROTATE, MODEL_TRANSLATE, MODEL_SCALE,
    VIEWPORT,
    NUM_MODES
  };
  static const Mode DEFAULT_MODE = VIEW_TRANSLATE;

  Viewer(AppWindow* appWindow);
  virtual ~Viewer();

  // A useful function that forces this widget to rerender. If you
  // want to render a new frame, do not call on_expose_event
  // directly. Instead call this, which will cause an on_expose_event
  // call when the time is right.
  void invalidate();

  // *** Fill in these functions (in viewer.cpp) ***

  // Set the parameters of the current perspective projection using
  // the semantics of gluPerspective().
  void set_perspective(double fov, double aspect,
                       double near, double far);

  // Restore all the transforms and perspective parameters to their
  // original state. Set the viewport to its initial size.
  void reset_view();
  void reset_perspective_screen();

  void set_mode(Mode mode);
  Mode get_mode();

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

private:
  // Clip in homogenous coordinates, before the lines have been divided/homogenized.
  bool homogenousClip(LineSegment4D& line);
  void renderHomogenousLines(std::vector<LineSegment4D> linesSegments);
  void handleViewChange(Vector3D& p);
  void reset_window_label();
  Matrix4x4 NDCToScreenMatrix();

  AppWindow* appWindow;

  Node* rootNode; // Where perspective matrix is set (non-homogenized homogenous coordinates).
  Node* worldNode; // Where view matrix is set (world coordinates).
  Node* modelNode; // Where model translation and rotation are set (model coordinates).
  Cube* cube; // Where model scaling is set.

  int lastMouseX;
  bool axisActive[3];
  Mode mode;
  double fovDegrees;
  double near, far;

  Point2D viewportTL, viewportBR;
  Point2D newViewportPos;
  bool firstConfig;
};

#endif
