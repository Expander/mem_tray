#!/usr/bin/env python

import gtk, psutil, gobject

def get_free_mem_frac():
    return psutil.virtual_memory().percent * 0.01

def create_xpm(lines):
    thresh = int((1 - get_free_mem_frac()) * lines)

    xpm = [str(lines) + " " + str(lines) + " 3 1",
           "  c None",
           ". c #000000",
           "* c #00FF00"]

    for y in range(0, thresh):
        xpm.append("..............................")
    for y in range(thresh, lines):
        xpm.append("******************************")

    return xpm

class MemTray(object):
    def __init__(self):
        self.lines = 30
        self.update_interval = 2 # seconds
        self.statusicon = gtk.StatusIcon()

    def update(self):
        self.statusicon.set_from_pixbuf(
            gtk.gdk.pixbuf_new_from_xpm_data(create_xpm(self.lines)))
        return True

    def tray(self):
        self.update()
        gobject.timeout_add(1000 * self.update_interval, self.update)
        gtk.main()

if __name__ == "__main__":
    mt = MemTray()
    mt.tray()
