#include "viewer.hpp"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include "a2.hpp"
#include "draw.hpp"

#define MOUSE_TRANSLATE_FACTOR 0.002
#define MOUSE_ROTATE_FACTOR 0.002
#define MOUSE_SCALE_FACTOR 0.002
#define MOUSE_FOV_FACTOR 0.04
#define MOUSE_NEARFAR_FACTOR 0.002

#define MIN_FOV 5.0
#define MAX_FOV 160.0

Viewer::Viewer(AppWindow* appWindow): appWindow(appWindow) {
  Glib::RefPtr<Gdk::GL::Config> glconfig;

  // Ask for an OpenGL Setup with
  //  - red, green and blue component colour
  //  - a depth buffer to avoid things overlapping wrongly
  //  - double-buffered rendering to avoid tearing/flickering
  glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB   |
                                     Gdk::GL::MODE_DEPTH |
                                     Gdk::GL::MODE_DOUBLE);
  if (glconfig == 0) {
    // If we can't get this configuration, die
    std::cerr << "Unable to setup OpenGL Configuration!" << std::endl;
    abort();
  }

  // Accept the configuration
  set_gl_capability(glconfig);

  // Register the fact that we want to receive these events
  add_events(Gdk::BUTTON1_MOTION_MASK    |
             Gdk::BUTTON2_MOTION_MASK    |
             Gdk::BUTTON3_MOTION_MASK    |
             Gdk::BUTTON_PRESS_MASK      |
             Gdk::BUTTON_RELEASE_MASK    |
             Gdk::VISIBILITY_NOTIFY_MASK);

  firstConfig = true;

  axisActive[0] = false;
  axisActive[1] = false;
  axisActive[2] = false;

  mode = VIEW_TRANSLATE;
  fovDegrees = 30.0;
  near = 1.0;
  far = 10.0;

  // Construct scene graph (it's linear):
  // (rootNode, Node, perspective mat)
  // ->(worldNode, Gnomon, viewing mat)
  //   ->(modelNode, Gnomon, model trans/rot mat)
  //     ->(cube, Cube, model scaling mat)

  const Colour WHITE = Colour(1);
  const Colour RED = Colour(1, 0, 0);
  const Colour BLUE = Colour(0, 0, 1);

  cube = new Cube();
  cube->setColour(BLUE);

  Gnomon* modelGnomon = new Gnomon();
  modelGnomon->setColour(RED);
  modelNode = modelGnomon; // This gnomon will act as model node.

  modelNode->addChild(cube);

  Gnomon* worldGnomon = new Gnomon();
  worldGnomon->setColour(WHITE);
  worldNode = worldGnomon; // This gnomon will act as world node.

  worldNode->addChild(modelNode);

  rootNode = new Node();
  rootNode->addChild(worldNode);
}

Viewer::~Viewer() {
  // Will cascade delete everything else.
  delete rootNode;
  worldNode = NULL;
  modelNode = NULL;
  cube = NULL;
}

void Viewer::set_mode(Viewer::Mode mode) {
  this->mode = mode;
  reset_window_label();
}

Viewer::Mode Viewer::get_mode() {
  return mode;
}

void Viewer::invalidate() {
  // Force a re-render.
  Gtk::Allocation allocation = get_allocation();
  get_window()->invalidate_rect( allocation, false);
}

void Viewer::set_perspective(double fov, double aspect,
                             double near, double far) {
  Matrix4x4 m = perspective(fov, aspect, near, far);
  rootNode->setTransform(m);
}

void Viewer::reset_perspective_screen() {
  double aspect = ((double)get_width()) / get_height();
  double fovRadians = fovDegrees*M_PI/180.0;
  set_perspective(fovRadians, aspect, near, far);

  invalidate();
}

void Viewer::reset_window_label() {
  appWindow->redraw_label(mode, fovDegrees, near, far);
}

// TODO: Make this properly reset on startup after window settles...
void Viewer::reset_view() {
  reset_perspective_screen(); // Will invalidate.
  Matrix4x4 viewingMatrix = translation(Vector3D(0.0, 0.0, -5.0));
  worldNode->setTransform(viewingMatrix);
  modelNode->resetTransform();
  cube->resetTransform();

  // Setup default viewport.
  viewportTL = Point2D(
    get_width() * 0.05,
    get_height() * 0.05
  );
  viewportBR = Point2D(
    get_width() * 0.95,
    get_height() * 0.95
  );
}

Matrix4x4 Viewer::NDCToScreenMatrix() {
  const double W = viewportBR[0] - viewportTL[0];
  const double H = viewportBR[1] - viewportTL[1];
  return translation(Vector3D(viewportTL[0] + W/2.0, viewportTL[1] + H/2.0, 0.0))
    * scaling(Vector3D(W/2.0, -H/2.0, 1.0));
}

void Viewer::on_realize() {
  // Do some OpenGL setup.
  // First, let the base class do whatever it needs to
  Gtk::GL::DrawingArea::on_realize();

  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable)
    return;

  if (!gldrawable->gl_begin(get_gl_context()))
    return;

  gldrawable->gl_end();

  reset_view();
  // Redraw label first time.
  reset_window_label();
}

bool Viewer::on_expose_event(GdkEventExpose* event) {
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  draw_init(get_width(), get_height());

  //Matrix4x4 transformationMatrix = m_perspective * m_view * m_model;
  //cube->setTransform(transformationMatrix); // Convert to homogenous coordinates.

  std::vector<LineSegment4D> lineSegments = rootNode->getTransformedLineSegments();

  renderHomogenousLines(lineSegments);

  // Draw viewport box.
  const Colour BLACK(0);
  set_colour(BLACK);
  const Point2D viewportBL = Point2D(viewportTL[0], viewportBR[1]);
  const Point2D viewportTR = Point2D(viewportBR[0], viewportTL[1]);
  draw_line(viewportTL, viewportBL);
  draw_line(viewportBL, viewportBR);
  draw_line(viewportBR, viewportTR);
  draw_line(viewportTR, viewportTL);

  draw_complete();

  // Swap the contents of the front and back buffers so we see what we
  // just drew. This should only be done if double buffering is enabled.
  gldrawable->swap_buffers();

  gldrawable->gl_end();

  return true;
}

bool Viewer::homogenousClip(LineSegment4D& line) {
  const int X = 0;
  const int Y = 1;
  const int W = 3;

  const Point4D p[] = { line.getP1(), line.getP2() };

  // Coordinates with respect to planes in homogeneous coordinates.
  // Positive means on visible size.
  double borderCoords[2][6];
  for (int pi = 0; pi < 2; pi++) {
    borderCoords[pi][0] = p[pi][W] - near;     // w - n = 0 (front)
    borderCoords[pi][1] = far - p[pi][W];      // f - w = 0 (back)
    borderCoords[pi][2] = p[pi][W] + p[pi][X]; // w + x = 0(left)
    borderCoords[pi][3] = p[pi][W] - p[pi][X]; // w - x = 0 (left)
    borderCoords[pi][4] = p[pi][W] + p[pi][Y]; // w + y = 0(bottom)
    borderCoords[pi][5] = p[pi][W] - p[pi][Y]; // w - y = 0(top)
  }

  bool anyChanged = false;
  double truncateLower = 0.0;
  double truncateHigher = 1.0;
  for (int side = 0; side < 6; side++) {
    bool inside1 = borderCoords[0][side] > 0;
    bool inside2 = borderCoords[1][side] > 0;
    if (!inside1 && !inside2) {
      return false;
    } else if (inside1 && inside2) {
      continue;
    } else { // One in and one out.
      double a = borderCoords[0][side]/(borderCoords[0][side] - borderCoords[1][side]);
      if (inside2) { // Replace point 1.
        truncateLower = std::max(truncateLower, a);
      } else { // Replace point 2.
        truncateHigher = std::min(truncateHigher, a);
      }
      if (truncateLower >= truncateHigher) {
        return false;
      }
      anyChanged = true;
    }
  }

  if (anyChanged) {
    line = LineSegment4D(
      (1-truncateLower)*p[0] + truncateLower*p[1],
      (1-truncateHigher)*p[0] + truncateHigher*p[1],
      line.getColour()
    );
  }
  return true;
}

void Viewer::renderHomogenousLines(std::vector<LineSegment4D> lineSegments) {
  LOG("rendering!");

  const Matrix4x4 screenMatrix = NDCToScreenMatrix();

  for (std::vector<LineSegment4D>::iterator lineIt = lineSegments.begin(); lineIt != lineSegments.end(); lineIt++) {
    LineSegment4D& line = *lineIt;
    bool keep = homogenousClip(line);
    if (!keep) {
      continue;
    }
    Point3D p1 = screenMatrix * line.getP1().homogenize();
    Point3D p2 = screenMatrix * line.getP2().homogenize();
    LOG("draw_line " << p1 << ", " << p2);
    set_colour(line.getColour());
    draw_line(
      Point2D(p1[0], p1[1]),
      Point2D(p2[0], p2[1])
    );
  }
}

bool Viewer::on_configure_event(GdkEventConfigure* event) {
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  gldrawable->gl_end();

  if (firstConfig) {
    firstConfig = false;
    reset_view();
  } else {
    reset_perspective_screen();
  }

  return true;
}

void Viewer::handleViewChange(Vector3D& v) {
  const char axisLabels[] = {'x', 'y', 'z'};
  switch (mode) {
    case VIEW_ROTATE:
      for (int axis = 0; axis < 3; axis++) {
        if (v[axis] != 0.0) {
          worldNode->rotate(v[axis] * MOUSE_ROTATE_FACTOR, axisLabels[axis]);
        }
      }
      break;
    case VIEW_TRANSLATE:
      worldNode->translate(MOUSE_TRANSLATE_FACTOR * v);
      break;
    case VIEW_PERSPECTIVE:
      if (v[0] != 0.0) {
        fovDegrees += v[0] * MOUSE_FOV_FACTOR;
        if (fovDegrees < MIN_FOV) {
          fovDegrees = MIN_FOV;
        } else if (fovDegrees > MAX_FOV) {
          fovDegrees = MAX_FOV;
        }
      } else if (v[1] != 0.0) {
        near += v[1] * MOUSE_NEARFAR_FACTOR;
      } else if (v[2] != 0.0) {
        far += v[2] * MOUSE_NEARFAR_FACTOR;
      }
      // Redraw label because we changed near/far.
      reset_window_label();
      reset_perspective_screen();
      break;
    case MODEL_ROTATE:
      for (int axis = 0; axis < 3; axis++) {
        if (v[axis] != 0.0) {
          modelNode->rotate(v[axis] * MOUSE_ROTATE_FACTOR, axisLabels[axis]);
        }
      }
      break;
    case MODEL_TRANSLATE:
      modelNode->translate(MOUSE_TRANSLATE_FACTOR * v);
      break;
    case MODEL_SCALE:
      cube->scale(MOUSE_SCALE_FACTOR * v + Vector3D(1.0, 1.0, 1.0));
      break;
    default:
      break;
  }
}

bool Viewer::on_button_press_event(GdkEventButton* event) {
  const int axis = event->button - 1;
  lastMouseX = event->x;
  axisActive[axis] = true;
  if (mode == VIEWPORT) {
    newViewportPos = Point2D(event->x, event->y);
  }
  return true;
}

bool Viewer::on_button_release_event(GdkEventButton* event) {
  const int axis = event->button - 1;
  axisActive[axis] = false;
  if (mode == VIEWPORT) {
    double x2, y2;
    if (event->x < 0) {
      x2 = 0;
    } else if (event->x > get_width()) {
      x2 = get_width();
    } else {
      x2 = event->x;
    }
    if (event->y < 0) {
      y2 = 0;
    } else if (event->y > get_height()) {
      y2 = get_height();
    } else {
      y2 = event->y;
    }
    viewportTL = Point2D(
      std::min(newViewportPos[0], x2),
      std::min(newViewportPos[1], y2)
    );
    viewportBR = Point2D(
      std::max(newViewportPos[0], x2),
      std::max(newViewportPos[1], y2)
    );
    invalidate();
  }
  return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event) {
  const int diff = event->x - lastMouseX;
  lastMouseX = event->x;
  bool anyChange = false;
  Vector3D transAmount;
  for (int axis = 0; axis < 3; axis++) {
    if (axisActive[axis]) {
      anyChange = true;
      transAmount[axis] = diff;
    }
  }
  if (anyChange) {
    LOG("Mouse translation " << transAmount);
    LOG("Now matrix is " << worldNode->getTransform());
    handleViewChange(transAmount);
    invalidate();
  }
  return true;
}
