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

G_DEFINE_TYPE (SwfdecWindow, swfdec_window, G_TYPE_OBJECT)

/* global list of windows */
static GSList *windows = NULL;

static void
swfdec_window_finalize (GObject *object)
{
  G_OBJECT_CLASS (swfdec_window_parent_class)->finalize (object);

  windows = g_slist_remove (windows, object);
  if (windows == NULL)
    gtk_main_quit ();
}

static void
swfdec_window_class_init (SwfdecWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = swfdec_window_finalize;
}

static void
swfdec_window_init (SwfdecWindow *window)
{
  windows = g_slist_prepend (windows, window);
}

/**
 * swfdec_window_set_url:
 * @window: the window that should show the given URL
 * @url: URL to show. Must be a valid file:// or http:// URL in UTF-8.
 *
 * Sets the URL of @window to be  @url, if no URL was set on @indow before.
 *
 * Returns: %TRUE if the URL could be set, %FALSE if the window already shows a 
 *          movie.
 **/
gboolean
swfdec_window_set_url (SwfdecWindow *window, const char *url)
{
  GObject *o;

  g_return_val_if_fail (SWFDEC_IS_WINDOW (window), FALSE);
  g_return_val_if_fail (url != NULL, FALSE);

  if (window->player || window->error)
    return FALSE;

  window->loader = swfdec_gtk_loader_new (url);
  window->player = swfdec_gtk_player_new (NULL);
  swfdec_player_set_loader (window->player, window->loader);
  swfdec_gtk_player_set_playing (SWFDEC_GTK_PLAYER (window->player), TRUE);
  o = gtk_builder_get_object (window->builder, "player-widget");
  swfdec_gtk_widget_set_player (SWFDEC_GTK_WIDGET (o), window->player);

  return TRUE;
}

/**
 * swfdec_window_error:
 * @window: a window
 * @msg: an error message to display for the user
 *
 * Shows the given error message to the user and aborts playback.
 * This function may be called at any time, no matter the state of the window 
 * object.
 **/
void
swfdec_window_error (SwfdecWindow *window, const char *msg)
{
  /* NB: This function can be called during the construction process (like when
   * the UI file isn't found, so a window object may not even exist. */

  /* FIXME: disable playback related menu items */
  /* FIXME: output this in a saner way */
  g_printerr ("%s\n", msg);
  window->error = TRUE;
}

#define BUILDER_FILE DATADIR G_DIR_SEPARATOR_S "swfdec-gnome" G_DIR_SEPARATOR_S "swfdec-player.ui"
/**
 * swfdec_window_new:
 * @uri: a valid UTF-8 encoded URI.
 *
 * Opens a new window. If a URI was specified, it will be shown. Otherwise the
 * window will appear empty.
 *
 * Returns: The window that was just openend. You don't own a reference to it.
 **/
SwfdecWindow *
swfdec_window_new (const char *url)
{
  GError *error = NULL;
  SwfdecWindow *window;

  window = g_object_new (SWFDEC_TYPE_WINDOW, NULL);
  window->builder = gtk_builder_new ();
  gtk_builder_set_translation_domain (window->builder, GETTEXT_PACKAGE);
  if (!gtk_builder_add_from_file (window->builder, BUILDER_FILE, &error)) {
    swfdec_window_error (window, error->message);
    return window;
  }
  gtk_builder_connect_signals (window->builder, window);
  window->window = GTK_WIDGET (gtk_builder_get_object (window->builder, "player-window"));
  if (window->window == NULL) {
    swfdec_window_error (window, _("Internal error in the user interface defintion"));
    return window;
  }
  g_object_weak_ref (G_OBJECT (window->window), (GWeakNotify) g_object_unref, window);
  if (url != NULL) {
    swfdec_window_set_url (window, url);
  }

  return window;
}
