/* Swfdec Player
 * Copyright (C) 2006 Benjamin Otte <otte@gnome.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include "swfdec-window.h"

static char *
sanitize_url (const char *s)
{
  if (strstr (s, "://")) {
    return g_strdup (s);
  } else {
    char *dir, *full;
    if (g_path_is_absolute (s))
      return g_strconcat ("file://", s, NULL);
    dir = g_get_current_dir ();
    full = g_strconcat ("file://", dir, G_DIR_SEPARATOR_S, s, NULL);
    g_free (dir);
    return full;
  }
}

int 
main (int argc, char *argv[])
{
  SwfdecWindowSettings settings = { TRUE, TRUE };
  GError *error = NULL;
  char **filenames = NULL;
  SwfdecWindow *window;
  guint i;
  char *s;

  GOptionEntry options[] = {
    { "no-sound", 'n', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, &settings.sound, N_("don't play sound"), NULL },
    { "paused", 'p', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, &settings.playing, N_("start player paused"), NULL },
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &filenames, NULL, N_("[FILE ...]") },
    { NULL }
  };
  GOptionContext *ctx;

  g_set_application_name (_("Swfdec Flash Player"));
  ctx = g_option_context_new (NULL);
  g_option_context_set_translation_domain (ctx, GETTEXT_PACKAGE);
  g_option_context_add_main_entries (ctx, options, GETTEXT_PACKAGE);
  g_option_context_add_group (ctx, gtk_get_option_group (TRUE));
  g_option_context_parse (ctx, &argc, &argv, &error);
  g_option_context_free (ctx);

  if (error) {
    g_printerr (_("Error parsing command line arguments: %s\n"), error->message);
    g_error_free (error);
    return EXIT_FAILURE;
  }

  swfdec_init ();

  if (filenames == NULL) {
    /* open an empty window if no args */
    window = swfdec_window_new (NULL);
    swfdec_window_set_settings (window, &settings);
  } else {
    for (i = 0; filenames[i]; i++) {
      s = sanitize_url (filenames[i]);
      window = swfdec_window_new (s);
      g_free (s);
      swfdec_window_set_settings (window, &settings);
    }
    g_strfreev (filenames);
  }

  gtk_main ();

  return EXIT_SUCCESS;
}

