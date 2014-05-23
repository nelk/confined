#include "viewer.hpp"
#include <iostream>
#include <algorithm>
#include <limits>
#include <GL/gl.h>
#include <GL/glu.h>

#define MOUSE_ROT_FACTOR 0.2
#define MOUSE_SCALE_FACTOR 0.03
#define MAX_SCALE 1.5
#define MIN_SCALE 0.1
#define GRAVITY 0.4 // For particles.
#define FRAGMENT_SCALE 0.3
#define FRAGMENT_MOVE_FACTOR 0.1
#define MOUSE_NO_MOMENTUM_THRESHOLD 20

Viewer::Viewer(Game *game) : m_view_mode(WIREFRAME), m_game(game), m_scale(1.0), m_game_over(false), m_show_guide(true), m_lighting(true) {
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

  //glClearColor(0.7, 0.7, 1.0, 0.0);
  glClearColor(0.0, 0.0, 0.0, 1.0);

  // Lighting/shading.
  glShadeModel(GL_SMOOTH);
  GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  //GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat light_diffuse[] = { 0.4, 0.4, 0.4, 1.0 };
  GLfloat mat_specular[] = { 0.1, 0.1, 0.1, 1.0 };
  //GLfloat mat_shininess[] = { 0.0 };

  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  //glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

  //glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
  //glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0005);
  //glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.005);

  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);

  glEnable(GL_DEPTH_TEST);

  gldrawable->gl_end();
}

bool Viewer::on_expose_event(GdkEventExpose* event) {
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (m_lighting) {
    glEnable(GL_LIGHTING);
  } else {
    glDisable(GL_LIGHTING);
  }

  // Modify the current projection matrix so that we move the
  // camera away from the origin.  We'll draw the game at the
  // origin, and we need to back up to see it.


  //glMatrixMode(GL_PROJECTION);
  //glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPushMatrix();

  GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  // Perform view movement on modelview matrix so that light moves with viewport.
  glTranslated(0.0, 0.0, -40.0);
  glRotated(m_rot[0], 1, 0, 0);
  glRotated(m_rot[1], 0, 1, 0);
  glRotated(m_rot[2], 0, 0, 1);
  glScaled(m_scale, m_scale, m_scale);

  // You'll be drawing unit cubes, so the game will have width
  // 10 and height 24 (game = 20, stripe = 4).  Let's translate
  // the game so that we can draw it starting at (0,0) but have
  // it appear centered in the window.
  glTranslated(-5.0, -12.0, 0.0);

  // Set polygon mode.
  if (m_view_mode == WIREFRAME) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Draw U-shaped game well.
  glColor3d(0.0, 0.0, 1.0);

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

  // Draw fragments (effect).
  glColor3d(1.0, 1.0, 1.0);
  for (std::list<Point3D>::iterator it = m_fragment_pos.begin(); it != m_fragment_pos.end(); it++) {
    glPushMatrix();
    glTranslated((*it)[0], (*it)[1], (*it)[2]);
    glScaled(FRAGMENT_SCALE, FRAGMENT_SCALE, FRAGMENT_SCALE);
    //std::cout << "(" << (*it)[0] << "," << (*it)[1] << "," << (*it)[2] << ")" << std::endl;
    drawCube();
    glPopMatrix();
  }

  // Draw game state.
  for (int r = 0; r < m_game->getHeight() + 4; r++) {
    for (int c = 0; c < m_game->getWidth(); c++) {
      int pieceId = m_game->get(r, c);
      if (pieceId == -1) {
        continue;
      }

      if (m_view_mode != MULTICOLOUR) {
        setColourForId(pieceId);
      }
      glPushMatrix();
      glTranslated(c, r, 0.0);
      drawCube(m_view_mode == MULTICOLOUR, m_game_over);
      glPopMatrix();
    }
  }

  // Draw piece guide.
  if (m_show_guide) {
    if (m_view_mode == WIREFRAME) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glColor4d(0.5, 0.5, 0.5, 0.5);
    std::vector<int> guide;
    m_game->getGuide(guide);
    std::vector<int>::const_iterator guide_piece_it = guide.begin();
    while (guide_piece_it != guide.end()) {
      glPushMatrix();
      int c = *guide_piece_it;
      ++guide_piece_it;
      int r = *guide_piece_it;
      ++guide_piece_it;
      glTranslated(c, r, 0.0);
      drawCube();
      glPopMatrix();
    }
  }

  glPopMatrix();

  // We pushed a matrix onto the PROJECTION stack earlier, we
  // need to pop it.

  //glMatrixMode(GL_PROJECTION);
  //glPopMatrix();

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
      glColor3d(0.0, 0.7647, 1.0);
      break;
    case 3:
      glColor3d(1.0, 1.0, 0.0);
      break;
    case 4:
      glColor3d(1.0, 0.0, 1.0);
      break;
    case 5:
      glColor3d(1.0, 0.647, 0.0);
      break;
    case 6:
      glColor3d(0.933, 0.5098, 0.933);
      break;
    case 7:
      glColor3d(0.3098, 0.8706, 0.5216);
      break;
    default:
      break;
  }
}

void Viewer::drawCube(bool differenColours, bool randomColours) {
  // Draw Cube, going around points ccw as per opengl standard.
  glBegin(GL_QUADS);

  int colourIds[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  int onColour = 0;
  if (randomColours) {
    std::random_shuffle(colourIds, colourIds + 8);
    if (!differenColours) {
      setColourForId(colourIds[0]);
    }
  }

  // Back.
  if (differenColours) setColourForId(colourIds[onColour++]);
  glNormal3d(0.0, 0.0, -1.0);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 0.0);
  glVertex3d(1.0, 1.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);

  // Front.
  if (differenColours) setColourForId(colourIds[onColour++]);
  glNormal3d(0.0, 0.0, 1.0);
  glVertex3d(0.0, 0.0, 1.0);
  glVertex3d(0.0, 1.0, 1.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(1.0, 0.0, 1.0);

  // Left.
  if (differenColours) setColourForId(colourIds[onColour++]);
  glNormal3d(-1.0, 0.0, 0.0);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);
  glVertex3d(0.0, 1.0, 1.0);
  glVertex3d(0.0, 0.0, 1.0);

  // Right.
  if (differenColours) setColourForId(colourIds[onColour++]);
  glNormal3d(1.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 1.0);
  glVertex3d(1.0, 1.0, 1.0);
  glVertex3d(1.0, 1.0, 0.0);

  // Bottom.
  if (differenColours) setColourForId(colourIds[onColour++]);
  glNormal3d(0.0, -1.0, 0.0);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(0.0, 0.0, 1.0);
  glVertex3d(1.0, 0.0, 1.0);
  glVertex3d(1.0, 0.0, 0.0);

  // Top.
  if (differenColours) setColourForId(colourIds[onColour++]);
  glNormal3d(0.0, 1.0, 0.0);
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

// TODO: Fix letting mouse go after moving and stopping - should not keep rotating.
bool Viewer::on_button_press_event(GdkEventButton* event) {
  m_last_mouse_x = event->x;
  m_mouse_button = event->button;
  m_last_time = event->time;
  m_last_delta_time = 0;
  m_last_delta_x = 0;
  m_currently_scaling = event->state & GDK_SHIFT_MASK;
  if (!m_currently_scaling) {
    m_rotv[m_mouse_button - 1] = 0;
  }
  return true;
}

bool Viewer::on_button_release_event(GdkEventButton* event) {
  guint32 delta_time = event->time - m_last_time;
  if (delta_time > MOUSE_NO_MOMENTUM_THRESHOLD) {
    return true;
  }
  if (!m_currently_scaling) {
    m_rotv[m_mouse_button - 1] = m_last_delta_x / m_last_delta_time * MOUSE_ROT_FACTOR;
  }
  return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event) {
  m_last_delta_x = event->x - m_last_mouse_x;
  m_last_mouse_x = event->x;
  m_last_delta_time = event->time - m_last_time;
  m_last_time = event->time;
  if (m_currently_scaling) {
    m_scale += m_last_delta_x * MOUSE_SCALE_FACTOR;
    // Clamp scale.
    if (m_scale < MIN_SCALE) {
      m_scale = MIN_SCALE;
    } else if (m_scale > MAX_SCALE) {
      m_scale = MAX_SCALE;
    }
  } else {
    m_rot[m_mouse_button - 1] += m_last_delta_x * MOUSE_ROT_FACTOR;
  }
  return true;
}

bool Viewer::refresh() {
  // Ties rotational velocity to refresh rate.
  // Alternatively I could check the milliseconds since last refresh.
  for (int i = 0; i < 3; i++) {
    m_rot[i] += m_rotv[i] * REFRESH_DELAY;
  }
  std::list<Point3D>::iterator pos_iter = m_fragment_pos.begin();
  std::list<Vector3D>::iterator vel_iter = m_fragment_vel.begin();
  while (pos_iter != m_fragment_pos.end() && vel_iter != m_fragment_vel.end()) {
    (*vel_iter)[1] -= GRAVITY;
    *pos_iter = *pos_iter + (FRAGMENT_MOVE_FACTOR * *vel_iter);
    if ((*pos_iter)[1] < -10) {
      pos_iter = m_fragment_pos.erase(pos_iter);
      vel_iter = m_fragment_vel.erase(vel_iter);
    } else {
      pos_iter++;
      vel_iter++;
    }
  }

  invalidate();
  return true;
}


void Viewer::resetView() {
  m_rot = Vector3D();
  m_rotv = Vector3D();
  m_scale = 1.0;
  m_lighting = true;
  m_show_guide = true;
}

double randZeroOne() {
  return ((double) rand()) / std::numeric_limits<int>::max();
}

void Viewer::rowsDestroyed(const std::vector<int> &removed_rows) {
  for (std::vector<int>::const_iterator r_it = removed_rows.begin(); r_it != removed_rows.end(); r_it++) {
    for (int c = 0; c < m_game->getWidth(); c++) {
      for (int i = 0; i < 4; i++) {
        m_fragment_pos.push_back(Point3D(c, *r_it, 0));
        m_fragment_vel.push_back(Vector3D(randZeroOne() * 4.0 - 2.0, randZeroOne() * 2.0 + 2.0, randZeroOne() * 4.0 - 2.0));
      }
    }
  }
}

