#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <hoedown/src/html.h>
#include <hoedown/src/buffer.h>
#include <gtksourceview/gtksource.h>

/**
 * Struct for data
 */
struct data {
    WebKitWebView *web_view;
    gchar *markdown;
    gchar *html;
};

/**
 * Quit GTK on exit
 */
void
app_exit(GtkWidget* widget, GtkWidget* window) {
    gtk_main_quit();
}

/**
 *
 */
void
render_markdown(GtkTextBuffer *textbuffer, struct data *notes) {
    // Create markdown renderer
    hoedown_renderer *renderer = hoedown_html_renderer_new(0, 0);

    // Add hoedown extensions
    unsigned int extensions = HOEDOWN_EXT_TABLES | HOEDOWN_EXT_UNDERLINE;

    // Create markdown empty document
    hoedown_document *document = hoedown_document_new(renderer, extensions, 16);

    // Create markdown buffer
    hoedown_buffer *html = hoedown_buffer_new(16);

    // Get the buffer iter
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(textbuffer, &start, &end);

    notes->markdown = gtk_text_buffer_get_text(
        textbuffer,
        &start,
        &end,
        TRUE
    );

    // get HTML from markdown
    hoedown_document_render(
        document,
        html,
        notes->markdown,
        gtk_text_buffer_get_char_count(textbuffer)
    );
    notes->html = hoedown_buffer_cstr(html);

    // Set HTML to web view
    if (WEBKIT_IS_WEB_VIEW(notes->web_view)) {
        webkit_web_view_load_html(
            notes->web_view,
            notes->html,
            NULL
        );
    }
}


int
main(int argc, char* argv[]) {
    // Initialize GTK+
    gtk_init(&argc, &argv);

    // Create struct
    struct data notes;

    // Create a main window
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(win), 700, 500);

    /**
     * Create GTKSourceView
     */
    // Create buffer
    GtkSourceLanguage *lang = gtk_source_language_manager_get_language(
        gtk_source_language_manager_get_default(),
        "markdown"
    );
    GtkSourceBuffer *source_buffer = gtk_source_buffer_new_with_language(lang);

    // Todo: Options dialog
    GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(
        gtk_source_style_scheme_manager_get_default(),
        "tango"
    );
    gtk_source_buffer_set_style_scheme(source_buffer, scheme);

    // Create view from buffer
    GtkWidget *source_view = gtk_source_view_new_with_buffer(source_buffer);

    // Todo: Options dialog
    // Set options
    gtk_source_view_set_show_line_numbers(source_view, TRUE);
    gtk_source_view_set_highlight_current_line(source_view, TRUE);
    gtk_source_view_set_insert_spaces_instead_of_tabs(source_view, TRUE);

    // TODO: Options dialog
    // Set the font
    PangoFontDescription *font_desc = pango_font_description_from_string("Ubuntu Mono 14");
    gtk_widget_override_font(source_view, font_desc);
    pango_font_description_free(font_desc);

    /**
     * Create GTK WebKit
     */
    notes.web_view = (WebKitWebView *) webkit_web_view_new();

    // Scrolling Windows
    // Source
    GtkWidget *source_window = gtk_scrolled_window_new(NULL, NULL);
    // Web
    GtkWidget *web_window = gtk_scrolled_window_new(NULL, NULL);

    // Policy set
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(source_window),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(web_window),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    //
    // Layout
    //
    // Hbox 50%
    //
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);

    // Add Source View
    gtk_container_add(GTK_CONTAINER(source_window), source_view);
    gtk_container_add(GTK_CONTAINER(hbox), source_window);

    // Add Web View
    gtk_container_add(GTK_CONTAINER(web_window), GTK_WIDGET(notes.web_view));
    gtk_container_add(GTK_CONTAINER(hbox), web_window);

    //
    // Add the hbox to window
    //
    gtk_container_add(GTK_CONTAINER(win), hbox);

    hoedown_renderer *renderer = hoedown_html_renderer_new(0, 0);
    hoedown_document *document = hoedown_document_new(renderer, 0, 16);
    hoedown_buffer *html = hoedown_buffer_new(16);
    hoedown_document_render(document, html, "<p></p>", 7);

    // Load a web page into the browser instance
    webkit_web_view_load_html(notes.web_view, hoedown_buffer_cstr(html), NULL);

    // Make sure that when the browser area becomes visible, it will get mouse
    // and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(notes.web_view));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(win);

    //
    // Hook up callbacks
    //
    g_signal_connect(win, "destroy", G_CALLBACK(app_exit), NULL);
    g_signal_connect(source_buffer, "changed", G_CALLBACK(render_markdown), &notes);

    // Run the main GTK+ event loop
    gtk_main();

    return 0;
}