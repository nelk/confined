#include "viewer.hpp"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include "a2.hpp"
#include "draw.hpp"

#define MOUSE_TRANSLATE_FACTOR 0.002
#define MOUSE_ROTATE_FACTOR 0.002
#define MOUSE_SCALE_FACTOR 0.002

Viewer::Viewer() {
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

  axisActive[0] = false;
  axisActive[1] = false;
  axisActive[2] = false;

  mode = VIEW_TRANSLATE;

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

void Viewer::reset_view() {
  //set_perspective(30.0 * M_PI/180.0, 1.0, 1.0, 10.0);
  double aspect = 16.0/9.0; //get_width() / get_height();
  set_perspective(M_PI/4.0, aspect, 1.0, 10.0);
  Matrix4x4 viewingMatrix = translation(Vector3D(0.0, 0.0, -5.0));
  worldNode->setTransform(viewingMatrix);
  modelNode->resetTransform();
  cube->resetTransform();
  //cube->scale(Vector3D(1.0, 1.0, 0.2));

  // Note that screen coordinates are flipped vertically.
  screenMatrix = translation(Vector3D(get_width()/2.0, get_height()/2.0, 0.0))
    * scaling(Vector3D(get_width()/4.0, -get_height()/4.0, 1.0));
  invalidate();
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
  // TEMP
  /*
  std::cout << (m_perspective * Point4D(0, 0, -1.0)).homogenize() << std::endl;
  std::cout << (m_perspective * Point4D(0, 0, -4.0)).homogenize() << std::endl;
  std::cout << (m_perspective * Point4D(0, 0, -10.0)).homogenize() << std::endl;
  std::cout << (m_perspective * Point4D(2.0, 0, -8.0)).homogenize() << std::endl;
  */
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

  draw_complete();

  // Swap the contents of the front and back buffers so we see what we
  // just drew. This should only be done if double buffering is enabled.
  gldrawable->swap_buffers();

  gldrawable->gl_end();

  return true;
}

bool Viewer::homogenousClip(LineSegment4D& line) {
  // TODO.
  double w1 = line.getP1()[3];
  double w2 = line.getP2()[3];
  if (w1 >= -0.001 && w1 <= 0.001) {
    return false;
  } else if (w2 >= -0.001 && w2 <= 0.001) {
    return false;
  }
  return true;
}

void Viewer::renderHomogenousLines(std::vector<LineSegment4D> lineSegments) {
  LOG("rendering!");
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
    /*
    draw_line(
      Point2D(p1[0]/p1[2], p1[1]/p1[2]),
      Point2D(p2[0]/p2[2], p2[1]/p2[2])
    );
    */
  }
}

/*
std::vector<Point4D> Viewer::createCube() {
  // Create Cube, going around points ccw as per opengl standard.

  std::vector<Point3D> points(12);

  // Back.
  //glNormal3d(0.0, 0.0, -1.0);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 0.0);
  glVertex3d(1.0, 1.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);
  draw_line();

  // Front.
  //glNormal3d(0.0, 0.0, 1.0);
  glVertex3d(0.0, 0.0, 1.0);
  glVertex3d(0.0, 1.0, 1.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(1.0, 0.0, 1.0);

  // Left.
  //glNormal3d(-1.0, 0.0, 0.0);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);
  glVertex3d(0.0, 1.0, 1.0);
  glVertex3d(0.0, 0.0, 1.0);

  // Right.
  //glNormal3d(1.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 1.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(1.0, 1.0, 0.0);

  // Bottom.
  //glNormal3d(0.0, -1.0, 0.0);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(0.0, 0.0, 1.0);
  glVertex3d(1.0, 0.0, 1.0);
  glVertex3d(1.0, 0.0, 0.0);

  // Top.
  //glNormal3d(0.0, 1.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);
  glVertex3d(1.0, 1.0, 0.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(0.0, 1.0, 1.0);
}
*/


bool Viewer::on_configure_event(GdkEventConfigure* event) {
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  gldrawable->gl_end();

  // TODO: Fix bug - somehow gets into state where rotation acts strangely - doesn't maintain shape of cube...?

  // TODO
  //double aspect = get_width() / get_height();
  double aspect = 16.0/9.0; //get_width() / get_height();
  set_perspective(M_PI/4.0, aspect, 1.0, 10.0);
  screenMatrix = translation(Vector3D(get_width()/2.0, get_height()/2.0, 0.0))
    * scaling(Vector3D(get_width()/4.0, -get_height()/4.0, 1.0));

  return true;
}

bool Viewer::on_button_press_event(GdkEventButton* event) {
  //std::cerr << "Stub: Button " << event->button << " pressed" << std::endl;
  const int axis = event->button - 1;
  lastMouseX = event->x;
  axisActive[axis] = true;
  return true;
}

bool Viewer::on_button_release_event(GdkEventButton* event) {
  //std::cerr << "Stub: Button " << event->button << " released" << std::endl;
  const int axis = event->button - 1;
  axisActive[axis] = false;
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
      // TODO
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

bool Viewer::on_motion_notify_event(GdkEventMotion* event) {
  //std::cerr << "Stub: Motion at " << event->x << ", " << event->y << std::endl;

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
  //event->time
  return true;
}
