/* Swfdec
 * Copyright (C) 2007 Benjamin Otte <otte@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>

#include "swfdec-window.h"

static void
menu_file_open_response (GtkFileChooser *chooser, gint response, SwfdecWindow *window)
{
  if (response == GTK_RESPONSE_ACCEPT) {
    char *target = gtk_file_chooser_get_filename (chooser);
    if (target != NULL) {
      char *url = g_strconcat ("file://", target, NULL);

      if (!swfdec_window_set_url (window, url)) {
	SwfdecWindow *new = swfdec_window_new (url);
	swfdec_window_set_settings (new, &window->settings);
      }
      g_free (url);
      g_free (target);
    }
  }
    
  gtk_widget_destroy (GTK_WIDGET (chooser));
  return;
}

void
menu_file_open (GtkAction *action, SwfdecWindow *window)
{
  GtkWidget *chooser;

  chooser = gtk_file_chooser_dialog_new (_("Select a file to play"),
      GTK_WINDOW (window->window), 
      GTK_FILE_CHOOSER_ACTION_OPEN, 
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT);

  g_object_ref (window);
  g_signal_connect (chooser, "response", G_CALLBACK (menu_file_open_response), window);
  g_object_weak_ref (G_OBJECT (chooser), (GWeakNotify) g_object_unref, window);
  gtk_window_present (GTK_WINDOW (chooser));
}

void
menu_file_play (GtkToggleAction *action, SwfdecWindow *window)
{
  window->settings.playing = gtk_toggle_action_get_active (action);
  if (window->player) {
    swfdec_gtk_player_set_playing (SWFDEC_GTK_PLAYER (window->player), 
	window->settings.playing);
  }
}

void
menu_file_mute (GtkToggleAction *action, SwfdecWindow *window)
{
  window->settings.sound = !gtk_toggle_action_get_active (action);
  if (window->player) {
    swfdec_gtk_player_set_audio_enabled (SWFDEC_GTK_PLAYER (window->player),
	window->settings.sound);
  }
}

void
menu_file_close (GtkAction *action, SwfdecWindow *window)
{
  gtk_widget_destroy (window->window);
}

void
menu_help_about (GtkAction *action, SwfdecWindow *window)
{
  static const char *authors[] = {
    "Benjamin Otte <otte@gnome.org>",
    "Pekka Lampila <pekka.lampila@iki.fi>",
    NULL,
  };
  static const char *artists[] = {
    "Cristian Grada <krigenator@gmail.com>",
    NULL
  };

  gtk_show_about_dialog (NULL, 
      "logo-icon-name", "swfdec",
      "authors", authors,
      "artists", artists,
      "comments", "Play Adobe Flash files",
      "version", VERSION,
      "website", "http://swfdec.freedesktop.org/",
      NULL);
}

gboolean
main_window_destroy_cb (GtkWidget *widget, GdkEvent *event, SwfdecWindow *window)
{
  return FALSE;
}
