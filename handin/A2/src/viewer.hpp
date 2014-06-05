#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <gtkmm.h>
#include <gtkglmm.h>
#include "algebra.hpp"
#include "scene.hpp"
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

  // Set the parameters of the current perspective projection using
  // the semantics of gluPerspective().
  void set_perspective(double fov, double aspect,
                       double near, double far);

  // Restore all the transforms and perspective parameters to their
  // original state. Set the viewport to its initial size.
  void reset_view();

  // Reset perspective to current window's aspect ratio and the current fov, near, and far parameters.
  void reset_perspective_screen();

  // Set and get the mode of interaction.
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
  // Returns false if the line should not be rendered at all. Otherwise the input parameter is mutated to be the clipped line.
  bool homogenousClip(LineSegment4D& line);

  // Given lines in homogeneous NDC, clip, transform to screen coordinates, and render.
  void renderHomogenousLines(std::vector<LineSegment4D> linesSegments);

  // Helper to modify view paramters from mouse delta (where buttons map to axis of p).
  void handleViewChange(Vector3D& p);

  void reset_window_label();
  void update_viewport(double x2, double y2);

  // Create this transformation matrix from member parameter variables.
  Matrix4x4 NDCToScreenMatrix();

  // Pointer to parent app window.
  AppWindow* appWindow;

  // Nodes in scene graph that hold transformations int he matrix stack.
  Node* rootNode; // Where perspective matrix is set (non-homogenized homogenous coordinates).
  Node* worldNode; // Where view matrix is set (world coordinates).
  Node* modelNode; // Where model translation and rotation are set (model coordinates).
  Cube* cube; // Where model scaling is set.

  // Variables for mouse input.
  int lastMouseX;
  bool axisActive[3];

  // View/interaction parameters.
  Mode mode;
  bool firstConfig;
  double fovDegrees;
  double near, far;
  Point2D viewportTL, viewportBR;
  Point2D newViewportPos;
};

#endif
