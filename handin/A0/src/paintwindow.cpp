
#include "paintwindow.hpp"
#include <gdkmm/color.h>
#include <gtkmm/radiomenuitem.h>

PaintWindow::PaintWindow() {
  set_title("488 Paint");

  using Gtk::Menu_Helpers::MenuElem;

  // Set up the application menu.
  Gtk::MenuItem *clear = Gtk::manage(new Gtk::MenuItem("_Clear", true));
  Gtk::MenuItem *quit = Gtk::manage(new Gtk::MenuItem("_Quit", true));

  clear->signal_activate().connect(sigc::mem_fun(*this, &PaintWindow::clear));
  quit->signal_activate().connect(sigc::mem_fun(*this, &PaintWindow::hide));

  clear->add_accelerator("activate", get_accel_group(), 'c', (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  clear->add_accelerator("activate", get_accel_group(), 'c', Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
  quit->add_accelerator("activate", get_accel_group(), 'q', (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  quit->add_accelerator("activate", get_accel_group(), 'q', Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);

  m_menu_app.items().push_back(*clear);
  m_menu_app.items().push_back(*quit);

  // Set up the tools menu
  //
  // We're going to be connecting a bunch of menu entries to the same
  // function. So, we put a slot corresponding to that function in
  // mode_slot.
  //
  // The type shows that this slot returns void (nothing, and takes
  // one argument, a PaintCanvas::Mode.
  sigc::slot1<void, PaintCanvas::Mode> mode_slot = 
    sigc::mem_fun(m_canvas, &PaintCanvas::set_mode);

  // Now we set up the actual tools. SigC::bind takes a slot and makes
  // a new slot with one fewer parameter than the one passed to it,
  // and "binds in" the constant value passed.
  //
  // In our case we take the reference to PaintCanvas::set_mode we
  // declared above, and bind in the appropriate mode, making a slot
  // that calls set_mode with the given mode (line/oval/rectangle).

  Gtk::RadioMenuItem::Group tool_group;
  Gtk::RadioMenuItem *rb_line = Gtk::manage(new Gtk::RadioMenuItem(tool_group, "_Line", true));
  Gtk::RadioMenuItem *rb_oval = Gtk::manage(new Gtk::RadioMenuItem(tool_group, "_Oval", true));
  Gtk::RadioMenuItem *rb_rectangle = Gtk::manage( new Gtk::RadioMenuItem(tool_group, "_Rectangle", true));

  rb_line->add_accelerator("activate", get_accel_group(), 'l', (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  rb_line->add_accelerator("activate", get_accel_group(), 'l', Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
  rb_oval->add_accelerator("activate", get_accel_group(), 'o', (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  rb_oval->add_accelerator("activate", get_accel_group(), 'o', Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
  rb_rectangle->add_accelerator("activate", get_accel_group(), 'r', (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  rb_rectangle->add_accelerator("activate", get_accel_group(), 'r', Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);

  rb_line->signal_activate().connect(sigc::bind( mode_slot, PaintCanvas::DRAW_LINE));
  rb_oval->signal_activate().connect(sigc::bind( mode_slot, PaintCanvas::DRAW_OVAL));
  rb_rectangle->signal_activate().connect(sigc::bind( mode_slot, PaintCanvas::DRAW_RECTANGLE));

  m_menu_tools.items().push_back(*rb_line);
  m_menu_tools.items().push_back(*rb_oval);
  m_menu_tools.items().push_back(*rb_rectangle);

  // Colour menu.
  sigc::slot1<void, Gdk::Color> colour_slot =
    sigc::mem_fun(m_canvas, &PaintCanvas::set_colour);

  Gdk::Color black, red, green, blue;
  black.set("black");
  red.set("red");
  green.set("green");
  blue.set("blue");

  Gtk::RadioMenuItem::Group colour_group;
  Gtk::RadioMenuItem *rb_black = Gtk::manage(new Gtk::RadioMenuItem(colour_group, "_Black", true));
  Gtk::RadioMenuItem *rb_red = Gtk::manage(new Gtk::RadioMenuItem(colour_group, "_Red", true));
  Gtk::RadioMenuItem *rb_green = Gtk::manage( new Gtk::RadioMenuItem(colour_group, "_Green", true));
  Gtk::RadioMenuItem *rb_blue = Gtk::manage( new Gtk::RadioMenuItem(colour_group, "_Blue", true));

  rb_black->signal_activate().connect(sigc::bind(colour_slot, black));
  rb_red->signal_activate().connect(sigc::bind(colour_slot, red));
  rb_green->signal_activate().connect(sigc::bind(colour_slot, green));
  rb_blue->signal_activate().connect(sigc::bind(colour_slot, blue));

  m_menu_colours.items().push_back(*rb_black);
  m_menu_colours.items().push_back(*rb_red);
  m_menu_colours.items().push_back(*rb_green);
  m_menu_colours.items().push_back(*rb_blue);

  // Set up the help menu
  m_menu_help.items().push_back(MenuElem("_Line Help",
        sigc::mem_fun(*this, &PaintWindow::help_line)));
  m_menu_help.items().push_back(MenuElem("_Oval Help",
        sigc::mem_fun(*this, &PaintWindow::help_oval)));
  m_menu_help.items().push_back(MenuElem("_Rectangle Help",
        sigc::mem_fun(*this, &PaintWindow::help_rectangle)));

  // Set up the menu bar
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Application", m_menu_app));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Tools", m_menu_tools));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Colours", m_menu_colours));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Help", m_menu_help));
  m_menubar.items().back().set_right_justified(true);

  // Pack in our widgets

  // First add the vertical box as our single "top" widget
  add(m_vbox);

  // Put the menubar on the top, and make it as small as possible
  m_vbox.pack_start(m_menubar, Gtk::PACK_SHRINK);

  // Put the canvas below the menubar. pack_start "grows" the widget
  // by default, so it'll take up the rest of the window.
  m_canvas.set_size_request(300, 300);
  m_vbox.pack_start(m_canvas);

  quit_button.set_label("Quit");
  m_vbox.pack_start(quit_button);
  quit_button.signal_clicked().connect(sigc::mem_fun(*this, &PaintWindow::hide));

  show_all();
}

void PaintWindow::help_dialog(const char *message) {
  Gtk::MessageDialog dialog(*this, message, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
  dialog.run();
}

void PaintWindow::help_line() {
  const char* message =
    "Drawing a Line\n"
    "\n"
    "To draw a line, press the left mouse button to mark the beginning of the line.  Drag the mouse to the end of the line and release the button.";
  help_dialog(message);
}

void PaintWindow::help_oval() {
  const char* message =
    "Drawing an Oval\n"
    "\n"
    "To draw an oval, press the left mouse button to mark one point.  Drag the mouse to the to form a rectangle, and release the button. An oval will be drawn within the rectangle.";
  help_dialog(message);
}

void PaintWindow::help_rectangle() {
  const char* message =
    "Drawing a Rectangle\n"
    "\n"
    "To draw a rectangle, press the left mouse button to mark one point.  Drag the mouse to the to form a rectangle, and release the button.";
  help_dialog(message);
}


void PaintWindow::clear() {
  m_canvas.clear();
}

