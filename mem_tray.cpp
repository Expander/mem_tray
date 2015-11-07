#include <gtk/gtk.h>
#include <limits>
#include <fstream>
#include <string>
#include <vector>

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

GdkPixbuf* create_pixbuf(
   unsigned width, unsigned height,
   const std::string& bg_color, const std::string& fg_color)
{
   const double free_mem_frac = get_free_mem_frac();
   const unsigned thresh = static_cast<unsigned>(free_mem_frac * height);

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
   void main();
   int check();

private:
   GtkStatusIcon* tray_icon;
   GdkPixbuf* pixbuf;
   std::string bg_color, fg_color;
   unsigned width, height;
   unsigned update_interval_in_seconds;
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

int Tray_icon::check()
{
   if (pixbuf)
      gdk_pixbuf_unref(pixbuf);
   pixbuf = create_pixbuf(width, height, bg_color, fg_color);
   gtk_status_icon_set_from_pixbuf(tray_icon, pixbuf);

   return 1;
}

void Tray_icon::main()
{
   check();
   gtk_status_icon_set_visible(tray_icon, TRUE);
   g_timeout_add_seconds(update_interval_in_seconds, GSourceFunc(&Tray_icon::check), this);
   gtk_main();
}

int main(int argc, char **argv) {
   gtk_init(&argc, &argv);

   Tray_icon tray_icon(30, 30, 2, "#000000", "#00FF00");
   tray_icon.main();

   return 0;
}
