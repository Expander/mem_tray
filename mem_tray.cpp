#include <gtk/gtk.h>
#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void print_usage(const char* program_name)
{
   std::cout <<
      "Usage: " << program_name << " [options]\n"
      "Options:\n"
      "  --bg-color=<color> background color\n"
      "                     Example: --bg-color=\"#000000\"\n"
      "                              --bg-color=None\n"
      "  --fg-color=<color> foreground color\n"
      "                     Example: --fg-color=\"#FF0000\"\n"
      "  --height=<value>   height of tray icon\n"
      "  --width=<value>    width of tray icon\n"
      "  --help,-h          print this help message"
             << std::endl;
}

/// test whether `str' starts with `prefix'
bool starts_with(const std::string& str, const std::string& prefix)
{
   return !str.compare(0, prefix.size(), prefix);
}

/// returns fraction of free memory
double get_free_mem_frac()
{
   unsigned long mem_total = 0;
   unsigned long mem_free  = 0;

   std::string token;
   std::ifstream file("/proc/meminfo");
   while (file >> token) {
      if (token == "MemTotal:") {
         file >> mem_total;
      }
      if (token == "MemFree:") {
         file >> mem_free;
      }
      // ignore rest of the line
      file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
   }

   const double free_mem_frac =  1. * mem_free / mem_total;

   if (free_mem_frac < 0.)
      return 0.;

   if (free_mem_frac > 1.)
      return 1.;

   return free_mem_frac;
}

/// returns pixbuf which displays a given fraction
GdkPixbuf* create_pixbuf(
   double frac, unsigned width, unsigned height,
   const std::string& bg_color, const std::string& fg_color)
{
   const unsigned thresh = static_cast<unsigned>(frac * height);

   std::string empty(width, '.');
   std::string full(width, '*');
   std::vector<std::string> offset;
   offset.push_back(std::to_string(width) + ' ' + std::to_string(height) + " 2 1");
   offset.push_back(". c " + bg_color);
   offset.push_back("* c " + fg_color);

   const char** xpm = static_cast<const char**>(
      malloc(sizeof(const char*) * (height + offset.size())));

   for (unsigned i = 0; i < offset.size(); i++)
      xpm[i] = offset[i].c_str();

   for (unsigned y = offset.size(); y < thresh + offset.size(); y++)
      xpm[y] = empty.c_str();

   for (unsigned y = thresh + offset.size(); y < height + offset.size(); y++)
      xpm[y] = full.c_str();

   GdkPixbuf* pixbuf = gdk_pixbuf_new_from_xpm_data(xpm);

   free(xpm);

   return pixbuf;
}

class Tray_icon {
public:
   Tray_icon(unsigned width_, unsigned height_, unsigned update_interval_in_seconds_,
             const std::string& bg_color_, const std::string& fg_color_);
   ~Tray_icon();
   void main(); ///< display tray icon

private:
   GtkStatusIcon* tray_icon;
   GdkPixbuf* pixbuf;
   std::string bg_color, fg_color;
   unsigned width, height;
   unsigned update_interval_in_seconds;

   int update(); ///< update tray icon
};

Tray_icon::Tray_icon(unsigned width_, unsigned height_, unsigned update_interval_in_seconds_,
                     const std::string& bg_color_, const std::string& fg_color_)
   : tray_icon(gtk_status_icon_new())
   , pixbuf(NULL)
   , bg_color(bg_color_)
   , fg_color(fg_color_)
   , width(width_)
   , height(height_)
   , update_interval_in_seconds(update_interval_in_seconds_)
{
}

Tray_icon::~Tray_icon()
{
   if (pixbuf)
      gdk_pixbuf_unref(pixbuf);
}

int Tray_icon::update()
{
   if (pixbuf)
      gdk_pixbuf_unref(pixbuf);

   pixbuf = create_pixbuf(get_free_mem_frac(), width, height, bg_color, fg_color);
   gtk_status_icon_set_from_pixbuf(tray_icon, pixbuf);

   return 1;
}

void Tray_icon::main()
{
   update();
   gtk_status_icon_set_visible(tray_icon, TRUE);
   g_timeout_add_seconds(update_interval_in_seconds, GSourceFunc(&Tray_icon::update), this);
   gtk_main();
}

int main(int argc, char **argv)
{
   // default values
   unsigned width = 30, height = 30, update_interval_in_seconds = 2;
   std::string fg_color = "#00FF00", bg_color = "#000000";

   for (int i = 1; i < argc; ++i) {
      const std::string option(argv[i]);

      if (starts_with(option, "--bg-color=")) {
         bg_color = option.substr(11);
         continue;
      }
      if (starts_with(option, "--fg-color=")) {
         fg_color = option.substr(11);
         continue;
      }
      if (starts_with(option, "--height=")) {
         height = std::stoi(option.substr(9));
         continue;
      }
      if (starts_with(option, "--width=")) {
         width = std::stoi(option.substr(8));
         continue;
      }
      if (option == "--help" || option == "-h") {
         print_usage(argv[0]);
         exit(EXIT_SUCCESS);
      }

      std::cerr << "Error: unrecognized command line option: " << option << '\n';
      exit(EXIT_FAILURE);
   }

   gtk_init(0, NULL);
   Tray_icon tray_icon(width, height, update_interval_in_seconds, bg_color, fg_color);
   tray_icon.main();

   return 0;
}
