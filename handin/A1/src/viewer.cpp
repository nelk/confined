#include "viewer.hpp"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>

Viewer::Viewer(Game *game) : m_view_mode(WIREFRAME), m_game(game) {
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
}

void Viewer::invalidate() {
  //Force a rerender
  Gtk::Allocation allocation = get_allocation();
  get_window()->invalidate_rect(allocation, false);

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

  // Just enable depth testing and set the background colour.
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.7, 0.7, 1.0, 0.0);

  gldrawable->gl_end();
}

bool Viewer::on_expose_event(GdkEventExpose* event) {
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  // Clear the screen

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Modify the current projection matrix so that we move the 
  // camera away from the origin.  We'll draw the game at the
  // origin, and we need to back up to see it.

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glTranslated(0.0, 0.0, -40.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Not implemented: set up lighting (if necessary)

  // Not implemented: scale and rotate the scene

  // You'll be drawing unit cubes, so the game will have width
  // 10 and height 24 (game = 20, stripe = 4).  Let's translate
  // the game so that we can draw it starting at (0,0) but have
  // it appear centered in the window.
  glTranslated(-5.0, -12.0, 0.0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glColor3d(0.3, 0.1, 0.8);

  // Draw U-shaped game well.
  // Sides.
  for (int r = 0; r < m_game->getHeight(); r++) {
    glPushMatrix();
    glTranslated(-1, r, 0.0);
    drawCube();
    glTranslated(m_game->getWidth() + 1, 0.0, 0.0);
    drawCube();
    glPopMatrix();
  }
  // Bottom.
  for (int c = -1; c < m_game->getWidth() + 1; c++) {
    glPushMatrix();
    glTranslated(c, -1, 0.0);
    drawCube();
    glPopMatrix();
  }

  /*
  glBegin(GL_TRIANGLES);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);

  glVertex3d(9.0, 0.0, 0.0);
  glVertex3d(10.0, 0.0, 0.0);
  glVertex3d(10.0, 1.0, 0.0);

  glVertex3d(0.0, 19.0, 0.0);
  glVertex3d(1.0, 20.0, 0.0);
  glVertex3d(0.0, 20.0, 0.0);

  glVertex3d(10.0, 19.0, 0.0);
  glVertex3d(10.0, 20.0, 0.0);
  glVertex3d(9.0, 20.0, 0.0);
  glEnd();
  */

  // Draw game state.
  if (m_view_mode == WIREFRAME) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  glColor3d(0.0, 1.0, 0.0);

  for (int r = 0; r < m_game->getHeight() + 4; r++) {
    for (int c = 0; c < m_game->getWidth(); c++) {
      int pieceId = m_game->get(r, c);
      if (pieceId == -1) {
        continue;
      }

      if (m_view_mode == MULTICOLOUR) {
        setColourForId(pieceId);
      }
      glPushMatrix();
      glTranslated(c, r, 0.0);
      drawCube();
      glPopMatrix();
    }
  }

  // We pushed a matrix onto the PROJECTION stack earlier, we
  // need to pop it.

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Swap the contents of the front and back buffers so we see what we
  // just drew. This should only be done if double buffering is enabled.
  gldrawable->swap_buffers();

  gldrawable->gl_end();

  return true;
}

void Viewer::setColourForId(int id) {
  switch (id) {
    case 0:
      glColor3d(1.0, 0.0, 0.0);
      break;
    case 1:
      glColor3d(0.0, 1.0, 0.0);
      break;
    case 2:
      glColor3d(0.0, 0.0, 1.0);
      break;
    case 3:
      glColor3d(1.0, 1.0, 0.0);
      break;
    case 4:
      glColor3d(1.0, 0.0, 1.0);
      break;
    case 5:
      glColor3d(0.0, 1.0, 1.0);
      break;
    case 6:
      glColor3d(1.0, 1.0, 1.0);
      break;
    default:
      break;
  }
}

void Viewer::drawCube() {
  // Draw Cube, going arround points cw.
  glBegin(GL_QUADS);

  // Back.
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 0.0);
  glVertex3d(1.0, 1.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);

  // Front.
  glVertex3d(0.0, 0.0, 1.0);
  glVertex3d(0.0, 1.0, 1.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(1.0, 0.0, 1.0);

  // Left.
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);
  glVertex3d(0.0, 1.0, 1.0);
  glVertex3d(0.0, 0.0, 1.0);

  // Right.
  glVertex3d(1.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 1.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(1.0, 1.0, 0.0);

  // Bottom.
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(0.0, 0.0, 1.0);
  glVertex3d(1.0, 0.0, 1.0);
  glVertex3d(1.0, 0.0, 0.0);

  // Top.
  glVertex3d(0.0, 1.0, 0.0);
  glVertex3d(1.0, 1.0, 0.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(0.0, 1.0, 1.0);

  glEnd();
}

bool Viewer::on_configure_event(GdkEventConfigure* event) {
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

bool Viewer::on_button_press_event(GdkEventButton* event) {
  std::cerr << "Stub: Button " << event->button << " pressed" << std::endl;
  return true;
}

bool Viewer::on_button_release_event(GdkEventButton* event) {
  std::cerr << "Stub: Button " << event->button << " released" << std::endl;
  return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event) {
  std::cerr << "Stub: Motion at " << event->x << ", " << event->y << std::endl;
  return true;
}

