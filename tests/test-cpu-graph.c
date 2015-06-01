#include "rg-graph.h"
#include "rg-cpu-table.h"
#include "rg-line-renderer.h"

#include <stdlib.h>

#define CSS_DATA \
  "RgGraph {\n" \
  "  background-color: #f6f7f8;\n" \
  "  background-size: 8px 8px;\n" \
  "  background-image:repeating-linear-gradient(0deg, #f0f1f2, #f0f1f2 1px, transparent 1px, transparent 8px),repeating-linear-gradient(-90deg, #f0f1f2, #f0f1f2 1px, transparent 1px, transparent 8px);\n" \
  "}"

static gchar *colors[] = {
  "#73d216",
  "#f57900",
  "#3465a4",
  "#ef2929",
  "#75507b",
  "#ce5c00",
  "#c17d11",
  "#ce5c00",
};

int
main (int argc,
      char *argv[])
{
  guint samples = 2;
  guint seconds = 30;
  const GOptionEntry entries[] = {
    { "samples", 'm', 0, G_OPTION_ARG_INT, &samples, "Number of samples per second", "2" },
    { "seconds", 's', 0, G_OPTION_ARG_INT, &seconds, "Number of seconds to display", "60" },
    { NULL }
  };
  gint64 timespan;
  guint max_samples;
  GOptionContext *context;
  GtkWindow *window;
  RgGraph *graph;
  RgTable *table;
  RgRenderer *renderer;
  GtkCssProvider *provider;
  GError *error = NULL;
  gsize i;

  context = g_option_context_new ("- a simple cpu graph");
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_add_main_entries (context, entries, NULL);

  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  g_print ("%d samples per second over %d seconds.\n",
           samples, seconds);

  timespan = (gint64)seconds * G_USEC_PER_SEC;
  max_samples = seconds * samples;

  table = g_object_new (RG_TYPE_CPU_TABLE,
                        "timespan", timespan,
                        "max-samples", max_samples,
                        NULL);

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider, CSS_DATA, -1, NULL);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (), GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 600,
                         "default-height", 325,
                         "title", "CPU Graph",
                         NULL);

  graph = g_object_new (RG_TYPE_GRAPH,
                        "visible", TRUE,
                        "table", table,
                        NULL);

  for (i = 0; i < g_get_num_processors (); i++)
    {
      renderer = g_object_new (RG_TYPE_LINE_RENDERER,
                               "column", i,
                               "stroke-color", colors[i % G_N_ELEMENTS (colors)],
                               "line-width", 1.0,
                               NULL);
      rg_graph_add_renderer (graph, renderer);
      g_object_unref (renderer);
    }

  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (graph));

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (window);
  gtk_main ();

  return 0;
}