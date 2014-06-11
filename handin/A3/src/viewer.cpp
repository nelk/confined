#include "viewer.hpp"
#include "algebra.hpp"
#include "primitive.hpp"
#include <iostream>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>

const std::string Viewer::SCENE_ROOT_ID = "the_scene_root_reserved_id";

Viewer::Viewer(SceneNode* luaScene): mode(Viewer::DEFAULT_MODE) {
  // Add our own root to the scene which we can reset translations on.
  luaSceneRoot = luaScene;
  sceneRoot = new SceneNode(SCENE_ROOT_ID);
  sceneRoot->add_child(luaSceneRoot);

  // Hold default transform of root and post-multiply our rotation onto root when orienting model.
  defaultLuaRootTransform = luaSceneRoot->get_transform();

  controller = new Controller(this, sceneRoot, luaSceneRoot);

  Glib::RefPtr<Gdk::GL::Config> glconfig;

  // Ask for an OpenGL Setup with
  //  - red, green and blue component colour
  //  - a depth buffer to avoid things overlapping wrongly
  //  - double-buffered rendering to avoid tearing/flickering
  glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB |
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
}

Viewer::~Viewer() {
  // Nothing to do here right now.
  delete controller;
  delete sceneRoot;
}

void Viewer::invalidate() {
  // Force a rerender
  Gtk::Allocation allocation = get_allocation();
  get_window()->invalidate_rect( allocation, false);
}

void Viewer::setMode(Mode mode) {
  if (this->mode != mode) {
    this->mode = mode;
    invalidate();
  }
}

Viewer::Mode Viewer::getMode() {
  return mode;
}

void Viewer::reset(ResetType r) {
  Matrix4x4 identity;
  if (r == RESET_POSITION || r == RESET_ALL) {
    sceneRoot->set_transform(identity);
  }
  if (r == RESET_ORIENTATION || r == RESET_ALL) {
    luaSceneRoot->set_transform(defaultLuaRootTransform);
  }
  if (r == RESET_JOINTS || r == RESET_ALL) {
    // TODO
  }
  invalidate();
}

void Viewer::on_realize()
{
  // Do some OpenGL setup.
  // First, let the base class do whatever it needs to
  Gtk::GL::DrawingArea::on_realize();

  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable)
    return;

  if (!gldrawable->gl_begin(get_gl_context()))
    return;

  // Initialize gl draw lists.
  Sphere::init();

  // Lighting/shading.
  glEnable(GL_NORMALIZE); // Make OpenGL normalize normals.
  glShadeModel(GL_SMOOTH);
  GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
  GLfloat light_diffuse[] = { 0.6, 0.6, 0.6, 1.0 };

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

  //glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
  //glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0005);
  //glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.005);

  glEnable(GL_LIGHT0);
  //glEnable(GL_COLOR_MATERIAL);

  glClearColor( 0.4, 0.4, 0.4, 0.0 );
  glEnable(GL_DEPTH_TEST);

  gldrawable->gl_end();
}

void Viewer::applyViewTransform() {
  glTranslated(0.0, 0.0, -8.0);
}

bool Viewer::blink() {
  GeometryNode::togglePickHighlight();
  invalidate();
  return true;
}

bool Viewer::on_expose_event(GdkEventExpose* event) {
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  // Set up for perspective drawing
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, get_width(), get_height());
  gluPerspective(40.0, (GLfloat)get_width()/(GLfloat)get_height(), 0.1, 1000.0);
  applyViewTransform();


  // change to model view for drawing
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Clear framebuffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Setup parameters.
  // TODO: Toggle
  glEnable(GL_LIGHTING);
  //GLfloat light_position[] = { 5.0, 0.0, 0.0, 1.0 };
  GLfloat light_position[] = { 0.0, 0.0, 10.0, 1.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  // Draw scene.
  sceneRoot->walk_gl(false);

  if (mode == POSITION) {
    draw_trackball_circle();
  }

  // Swap the contents of the front and back buffers so we see what we
  // just drew. This should only be done if double buffering is enabled.
  gldrawable->swap_buffers();

  gldrawable->gl_end();

  return true;
}

bool Viewer::on_configure_event(GdkEventConfigure* event)
{
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  // Set up perspective projection, using current size and aspect
  // ratio of display

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, event->width, event->height);
  gluPerspective(40.0, (GLfloat)event->width/(GLfloat)event->height, 0.1, 1000.0);

  // Reset to modelview matrix mode

  glMatrixMode(GL_MODELVIEW);

  gldrawable->gl_end();

  return true;
}

bool Viewer::on_button_press_event(GdkEventButton* event)
{
  //std::cerr << "Stub: Button " << event->button << " pressed" << std::endl;
  controller->press((Controller::Button)(event->button - 1), event->x, event->y);
  return true;
}

bool Viewer::on_button_release_event(GdkEventButton* event)
{
  //std::cerr << "Stub: Button " << event->button << " released" << std::endl;
  controller->release((Controller::Button)(event->button - 1), event->x, event->y);
  return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event)
{
  //std::cerr << "Stub: Motion at " << event->x << ", " << event->y << std::endl;
  controller->move(event->x, event->y);
  return true;
}

void Viewer::draw_trackball_circle() {
  const int current_width = get_width();
  const int current_height = get_height();

  // Set up for orthogonal drawing to draw a circle on screen.
  // You'll want to make the rest of the function conditional on
  // whether or not we want to draw the circle this time around.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, current_width, current_height);
  glOrtho(0.0, (float)current_width, 
           0.0, (float)current_height, -0.1, 0.1);

  // change to model view for drawing
  glMatrixMode(GL_MODELVIEW);

  // Reset modelview matrix
  glLoadIdentity();

  // Draw a circle for use with the trackball
  glDisable(GL_LIGHTING);
  glEnable(GL_LINE_SMOOTH);
  glColor3f(1.0, 1.0, 1.0);
  double radius = current_width < current_height ?
    (float)current_width * 0.25 : (float)current_height * 0.25;
  glTranslated((float)current_width * 0.5, (float)current_height * 0.5, 0);
  glBegin(GL_LINE_LOOP);
  for(size_t i=0; i<40; ++i) {
    double cosine = radius * cos(i*2*M_PI/40);
    double sine = radius * sin(i*2*M_PI/40);
    glVertex2f(cosine, sine);
  }
  glEnd();
  glColor3f(0.0, 0.0, 0.0);
  glDisable(GL_LINE_SMOOTH);
}
