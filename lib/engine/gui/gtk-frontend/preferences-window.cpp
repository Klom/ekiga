 
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         pref_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   Additional code      : Miguel Rodríguez Pérez  <miguelrp@gmail.com>
 */


#include "config.h"

#include <gmpreferences.h>
#include <gmconf.h>

#include "preferences-window.h"
#include "default_devices.h"

#include "scoped-connections.h"

#include "gmwindow.h"
#include "codecsbox.h"

#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"

#include "device-lists.h"

#ifdef WIN32
#include "platform/winpaths.h"
#endif

typedef struct _GmPreferencesWindow
{
  ~_GmPreferencesWindow();

  GtkWidget *audio_codecs_list;
  GtkWidget *sound_events_list;
  GtkWidget *audio_player;
  GtkWidget *sound_events_output;
  GtkWidget *audio_recorder;
  GtkWidget *video_device;
  GtkWidget *iface;
  GtkWidget *fsbutton;
  boost::shared_ptr<Ekiga::VideoInputCore> videoinput_core;
  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core;
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core;
  Ekiga::scoped_connections connections;
  std::list<gpointer> notifiers;
} GmPreferencesWindow;

#define GM_PREFERENCES_WINDOW(x) (GmPreferencesWindow *) (x)

_GmPreferencesWindow::~_GmPreferencesWindow()
{
  for (std::list<gpointer>::iterator iter = notifiers.begin ();
       iter != notifiers.end ();
       ++iter)
    gm_conf_notifier_remove (*iter);
}

/* Declarations */

/* GUI Functions */


/* DESCRIPTION  : /
 * BEHAVIOR     : Frees a GmPreferencesWindow and its content.
 * PRE          : A non-NULL pointer to a GmPreferencesWindow structure.
 */
static void gm_pw_destroy (gpointer prefs_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a pointer to the private GmPrerencesWindow structure
 *                used by the preferences window GMObject.
 * PRE          : The given GtkWidget pointer must be a preferences window
 * 		  GMObject.
 */
static GmPreferencesWindow *gm_pw_get_pw (GtkWidget *preferences_window);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the sound events list of the preferences window.
 * PRE          :  /
 */
static void gm_prefs_window_sound_events_list_build (GtkWidget *prefs_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Adds an update button connected to the given callback to
 * 		  the given GtkBox.
 * PRE          : A valid pointer to the container widget where to attach
 *                the button, followed by a label, the callback, a
 *                tooltip and the alignment.
 */
static void gm_pw_add_update_button (GtkWidget *box,
                                     const char *label,
                                     GCallback func,
                                     const gchar *tooltip,
                                     gfloat valign,
                                     gpointer data);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the general settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_general_page (GtkWidget *prefs_window,
				     GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the interface settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_interface_page (GtkWidget *prefs_window,
				       GtkWidget *container);



/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the call options page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_call_options_page (GtkWidget *prefs_window,
					  GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the sound events settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_sound_events_page (GtkWidget *prefs_window,
					  GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the H.323 settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_h323_page (GtkWidget *prefs_window,
				  GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the SIP settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_sip_page (GtkWidget *prefs_window,
				 GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the audio devices settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_audio_devices_page (GtkWidget *prefs_window,
					   GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the video devices settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_video_devices_page (GtkWidget *prefs_window,
					   GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the codecs settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_audio_codecs_page (GtkWidget *prefs_window,
                                          GtkWidget *container);
static void gm_pw_init_video_codecs_page (GtkWidget *prefs_window,
                                          GtkWidget *container);



/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the refresh devices list button in the prefs.
 * BEHAVIOR     :  Redetects the devices and refreshes the menu.
 * PRE          :  /
 */
static void refresh_devices_list_cb (GtkWidget *widget,
				     gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a sound event in the list.
 * BEHAVIOR     :  It udpates the GtkFileChooser's filename to the config
 *                 value for the key corresponding to the currently
 *                 selected sound event.
 * PRE          :  /
 */
static void sound_event_clicked_cb (GtkTreeSelection *selection,
				    gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the play button in the sound events list.
 * BEHAVIOR     :  Plays the currently selected sound event using the 
 * 		   selected audio player and plugin through a GMSoundEvent.
 * PRE          :  The entry.
 */
static void sound_event_play_cb (GtkWidget *widget,
				 gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a sound event in the list and change the toggle.
 * BEHAVIOR     :  It udpates the config key associated with the currently
 *                 selected sound event so that it reflects the state of the
 *                 sound event (enabled or disabled) and also updates the list.
 * PRE          :  /
 */
static void sound_event_toggled_cb (GtkCellRendererToggle *cell,
				    gchar *path_str, 
				    gpointer data);


/* DESCRIPTION  :  This callback is called when the user selected a file
 *                 with the file selector button for the image
 * BEHAVIOR     :  Update of the config database.
 * PRE          :  /
 */
/*
static void image_filename_browse_cb (GtkWidget *widget,
				      gpointer data);
*/

/* DESCRIPTION  :  This callback is called when the user selected a file
 *                 for a sound event
 * BEHAVIOR     :  Update of the config database.
 * PRE          :  /
 */
static void audioev_filename_browse_cb (GtkWidget *widget,
					gpointer data);


/* DESCRIPTION  :  This callback is called when something changes in the sound
 *                 events list.
 * BEHAVIOR     :  It updates the events list widget.
 * PRE          :  A pointer to the prefs window GMObject.
 */
static void sound_events_list_changed_nt (gpointer id,
					  GmConfEntry *entry,
					  gpointer data);


/* DESCRIPTION  :  This callback is called by the preview-play button of the
 * 		   selected audio file in the audio file selector.
 * BEHAVIOR     :  GMSoundEv's the audio file.
 * PRE          :  /
 */
static void audioev_filename_browse_play_cb (GtkWidget *playbutton,
                                             gpointer data);

void 
gm_prefs_window_update_devices_list (GtkWidget *prefs_window);

/* Implementation */
static void
gm_pw_destroy (gpointer prefs_window)
{
  g_return_if_fail (prefs_window != NULL);

  delete ((GmPreferencesWindow *) prefs_window);
}


static GmPreferencesWindow *
gm_pw_get_pw (GtkWidget *preferences_window)
{
  g_return_val_if_fail (preferences_window != NULL, NULL);

  return GM_PREFERENCES_WINDOW (g_object_get_data (G_OBJECT (preferences_window), "GMObject"));
}


static void
gm_prefs_window_sound_events_list_build (GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter, selected_iter;

  bool enabled = FALSE;

  pw = gm_pw_get_pw (prefs_window);

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  if (gtk_tree_selection_get_selected (selection, &model, &selected_iter))
    path = gtk_tree_model_get_path (model, &selected_iter);

  gtk_list_store_clear (GTK_LIST_STORE (model));

  /* Sound on incoming calls */
  enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_incoming_call_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play sound on incoming calls"),
		      2, SOUND_EVENTS_KEY "incoming_call_sound",
		      3, SOUND_EVENTS_KEY "enable_incoming_call_sound",
                      4, "incoming_call_sound",
		      -1);

  enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_ring_tone_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play ring tone"),
		      2, SOUND_EVENTS_KEY "ring_tone_sound",
		      3, SOUND_EVENTS_KEY "enable_ring_tone_sound",
                      4, "ring_tone_sound",
		      -1);

  enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_busy_tone_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play busy tone"),
		      2, SOUND_EVENTS_KEY "busy_tone_sound",
		      3, SOUND_EVENTS_KEY "enable_busy_tone_sound",
		      4, "busy_tone_sound",
		      -1);

  enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_new_voicemail_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play sound for new voice mails"),
		      2, SOUND_EVENTS_KEY "new_voicemail_sound",
		      3, SOUND_EVENTS_KEY "enable_new_voicemail_sound",
		      4, "new_voicemail_sound",
		      -1);

  enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_new_message_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play sound for new instant messages"),
		      2, SOUND_EVENTS_KEY "new_message_sound",
		      3, SOUND_EVENTS_KEY "enable_new_message_sound",
		      4, "new_message_sound",
		      -1);

  if (!path)
    path = gtk_tree_path_new_from_string ("0");

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (pw->sound_events_list),
			    path, NULL, false);
  gtk_tree_path_free (path);
}


static void
gm_pw_add_update_button (GtkWidget *box,
                         const char *label,
                         GCallback func,
                         const gchar *tooltip,
                         gfloat valign,
                         gpointer data)
{
  GtkWidget* alignment = NULL;
  GtkWidget* button = NULL;

  /* Update Button */
  button = gtk_button_new_with_mnemonic (label);
  gtk_widget_set_tooltip_text (button, tooltip);

  alignment = gtk_alignment_new (1, valign, 0, 0);
  gtk_container_add (GTK_CONTAINER (alignment), button);
  gtk_container_set_border_width (GTK_CONTAINER (button), 6);

  gtk_box_pack_start (GTK_BOX (box), alignment, TRUE, TRUE, 0);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (func),
                    (gpointer) data);
}


static void
gm_pw_init_general_page (GtkWidget *prefs_window,
                         GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  GtkWidget *entry = NULL;

  /* Personal Information */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                _("Personal Information"), 2, 2);

  entry = gnome_prefs_entry_new (subsection, _("_Full name:"),
				 PERSONAL_DATA_KEY "full_name",
				 _("Enter your full name"), 0, 2, false);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);
}


static void
gm_pw_init_interface_page (GtkWidget *prefs_window,
                           GtkWidget *container)
{
  GtkWidget *subsection = NULL;

  /* Video Display */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                _("Video Display"), 1, 2);

  gnome_prefs_toggle_new (subsection, _("Place windows displaying video _above other windows"), VIDEO_DISPLAY_KEY "stay_on_top", _("Place windows displaying video above other windows during calls"), 0, 2);

  /* Network Settings */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                           _("Network Settings"), 2, 2);

  gnome_prefs_spin_new (subsection, _("Type of Service (TOS):"), PROTOCOLS_KEY "rtp_tos_field", _("The Type of Service (TOS) byte on outgoing RTP IP packets. This byte is used by the network to provide some level of Quality of Service (QoS). Default value 184 (0xB8) corresponds to Expedited Forwarding (EF) PHB as defined in RFC 3246."), 0.0, 255.0, 1.0, 0, 2, NULL, true);

  gnome_prefs_toggle_new (subsection, _("Enable network _detection"), NAT_KEY "enable_stun", _("Enable the automatic network setup resulting from the STUN test"), 1, 2);
}

static void
gm_pw_init_call_options_page (GtkWidget *prefs_window,
                              GtkWidget *container)
{
  GtkWidget *subsection = NULL;

  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                           _("Call Forwarding"), 3, 2);

  gnome_prefs_toggle_new (subsection, _("_Always forward calls to the given host"), CALL_FORWARDING_KEY "always_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the protocol settings"), 0, 2);

  gnome_prefs_toggle_new (subsection, _("Forward calls to the given host if _no answer"), CALL_FORWARDING_KEY "forward_on_no_answer", _("If enabled, all incoming calls will be forwarded to the host that is specified in the protocol settings if you do not answer the call"), 1, 2);

  gnome_prefs_toggle_new (subsection, _("Forward calls to the given host if _busy"), CALL_FORWARDING_KEY "forward_on_busy", _("If enabled, all incoming calls will be forwarded to the host that is specified in the protocol settings if you already are in a call or if you are in busy mode"), 2, 2);


  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                           _("Call Options"), 2, 3);

  /* Add all the fields */
  gnome_prefs_spin_new (subsection, _("Call forwarding delay (in seconds):"), CALL_OPTIONS_KEY "no_answer_timeout", _("Automatically reject or forward incoming calls if no answer is given after the specified amount of time (in seconds)"), 10.0, 299.0, 1.0, 0, 3, NULL, true);
  gnome_prefs_toggle_new (subsection, _("_Automatically answer incoming calls"), CALL_OPTIONS_KEY "auto_answer", _("If enabled, automatically answer incoming calls"), 1, 3);
}


static void
gm_pw_init_sound_events_page (GtkWidget *prefs_window,
                              GtkWidget *container)
{
  GmPreferencesWindow *pw= NULL;

  GtkWidget *button = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *subsection = NULL;
  GtkWidget *selector_hbox = NULL;
  GtkWidget *selector_playbutton = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;

  GtkCellRenderer *renderer = NULL;

  GtkFileFilter *filefilter = NULL;

  PStringArray devs;

  pw = gm_pw_get_pw (prefs_window);

  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                           _("Ekiga Sound Events"), 
                                           1, 1);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_grid_attach (GTK_GRID (subsection), vbox, 0, 0, 1, 1);

  /* The 3rd column will be invisible and contain the config key containing
     the file to play. The 4th one contains the key determining if the
     sound event is enabled or not. */
  list_store =
    gtk_list_store_new (5,
                        G_TYPE_BOOLEAN,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING);

  pw->sound_events_list =
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (pw->sound_events_list), TRUE);

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), pw->sound_events_list);
  gtk_container_set_border_width (GTK_CONTAINER (pw->sound_events_list), 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);


  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
                                                     renderer,
                                                     "active", 
                                                     0,
                                                     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  g_signal_connect (renderer, "toggled",
                    G_CALLBACK (sound_event_toggled_cb), 
                    GTK_TREE_MODEL (list_store));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Event"),
                                                     renderer,
                                                     "text",
                                                     1,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 325);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  pw->fsbutton =
    gtk_file_chooser_button_new (_("Choose a sound"),
                                 GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_box_pack_start (GTK_BOX (hbox), pw->fsbutton, TRUE, TRUE, 2);

  filefilter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filefilter, _("Wavefiles"));
#ifdef WIN32
  gtk_file_filter_add_pattern (filefilter, "*.wav");
#else
  gtk_file_filter_add_mime_type (filefilter, "audio/x-wav");
#endif
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (pw->fsbutton), filefilter);

  selector_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  selector_playbutton = gtk_button_new_with_label (_("Play"));
  gtk_box_pack_end (GTK_BOX (selector_hbox),
                    selector_playbutton, FALSE, FALSE, 0);
  gtk_widget_show (selector_playbutton);
  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (pw->fsbutton),
                                     selector_hbox);

  g_signal_connect (selector_playbutton, "clicked",
                    G_CALLBACK (audioev_filename_browse_play_cb),
                    (gpointer) prefs_window);

  g_signal_connect (pw->fsbutton, "selection-changed",
                    G_CALLBACK (audioev_filename_browse_cb),
                    (gpointer) prefs_window);

  g_signal_connect (selection, "changed",
                    G_CALLBACK (sound_event_clicked_cb),
                    (gpointer) pw->fsbutton);

  button = gtk_button_new_with_label (_("Play"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (sound_event_play_cb),
                    (gpointer) prefs_window);

  /* Place it after the signals so that we can make sure they are run if
     required */
  gm_prefs_window_sound_events_list_build (prefs_window);
}


static void
gm_pw_init_h323_page (GtkWidget *prefs_window,
                      GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  const gchar *capabilities [] =
    {_("String"),
      _("Tone"),
      _("RFC2833"),
      _("Q.931"),
      NULL};

  const gchar *roles [] =
    { _("Disable H.239 Extended Video"),
      _("Allow H.239 per Content Role Mask"),
      _("Force H.239 Presentation Role"),
      _("Force H.239 Live Role"),
      NULL };

  /* Add Misc Settings */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                           _("Misc Settings"), 2, 2);

  entry =
    gnome_prefs_entry_new (subsection, _("Forward _URI:"), H323_KEY "forward_host", _("The host where calls should be forwarded if call forwarding is enabled"), 1, 2, false);
  if (!g_strcmp0 (gtk_entry_get_text (GTK_ENTRY (entry)), ""))
    gtk_entry_set_text (GTK_ENTRY (entry), "h323:");

  /* Packing widget */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                _("Advanced Settings"), 5, 4);

  /* The toggles */
  gnome_prefs_toggle_new (subsection, _("Enable H.245 _tunneling"), H323_KEY "enable_h245_tunneling", _("This enables H.245 Tunneling mode. In H.245 Tunneling mode H.245 messages are encapsulated into the H.225 channel (port 1720). This saves one TCP connection during calls. H.245 Tunneling was introduced in H.323v2."), 0, 4);

  gnome_prefs_toggle_new (subsection, _("Enable _early H.245"), H323_KEY "enable_early_h245", _("This enables H.245 early in the setup"), 1, 4);

  gnome_prefs_toggle_new (subsection, _("Enable fast _start procedure"), H323_KEY "enable_fast_start", _("Connection will be established in Fast Start (Fast Connect) mode. Fast Start is a new way to start calls faster that was introduced in H.323v2."), 2, 4);

  gnome_prefs_toggle_new (subsection, _("Enable H.239 control"), H323_KEY "enable_h239", _("This enables H.239 capability for additional video roles."), 3, 4);

  gnome_prefs_int_option_menu_new (subsection, NULL, roles, H323_KEY "video_role", _("Select the H.239 Video Role"), 4);

  /* Packing widget */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                _("DTMF Mode"), 1, 4);

  gnome_prefs_int_option_menu_new (subsection, _("_Send DTMF as:"), capabilities, H323_KEY "dtmf_mode", _("Select the mode for DTMFs sending"), 0);
}


static void
gm_pw_init_sip_page (GtkWidget *prefs_window,
                     GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  const gchar *capabilities [] =
    {
      _("RFC2833"),
      _("INFO"),
      NULL
    };

  /* Add Misc Settings */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                           _("Misc Settings"), 2, 2);

  gnome_prefs_entry_new (subsection, _("_Outbound proxy:"), SIP_KEY "outbound_proxy_host", _("The SIP Outbound Proxy to use for outgoing calls"), 0, 2, false);

  entry =
    gnome_prefs_entry_new (subsection, _("Forward _URI:"), SIP_KEY "forward_host", _("The host where calls should be forwarded if call forwarding is enabled"), 1, 2, false);
  if (!g_strcmp0 (gtk_entry_get_text (GTK_ENTRY (entry)), ""))
    gtk_entry_set_text (GTK_ENTRY (entry), "sip:");

  /* Packing widget */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                _("DTMF Mode"), 1, 1);

  gnome_prefs_int_option_menu_new (subsection, _("_Send DTMF as:"), capabilities, SIP_KEY "dtmf_mode", _("Select the mode for DTMFs sending"), 0);
}


static void
gm_pw_init_audio_devices_page (GtkWidget *prefs_window,
                               GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  PStringArray devs;
  gchar **array = NULL;
  GmPreferencesWindow *pw = NULL;

  pw = gm_pw_get_pw (prefs_window);

  /* Add all the fields */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                           _("Audio Devices"), 4, 3);

  /* Add all the fields for the audio manager */
  std::vector <std::string> device_list;

  get_audiooutput_devices (pw->audiooutput_core, device_list);
  array = vector_of_string_to_array (device_list);
  pw->sound_events_output =
    gnome_prefs_string_option_menu_new (subsection, _("Ringing device:"), (const gchar **)array, SOUND_EVENTS_KEY "output_device", _("Select the ringing audio device to use"), 0, DEFAULT_AUDIO_DEVICE_NAME);
  pw->audio_player =
    gnome_prefs_string_option_menu_new (subsection, _("Output device:"), (const gchar **)array, AUDIO_DEVICES_KEY "output_device", _("Select the audio output device to use"), 1, DEFAULT_AUDIO_DEVICE_NAME);
  g_free (array);

  /* The recorder */
  get_audioinput_devices (pw->audioinput_core, device_list);
  array = vector_of_string_to_array (device_list);
  pw->audio_recorder =
    gnome_prefs_string_option_menu_new (subsection, _("Input device:"), (const gchar **)array, AUDIO_DEVICES_KEY "input_device", _("Select the audio input device to use"), 2, DEFAULT_AUDIO_DEVICE_NAME);
  g_free (array);

  /* That button will refresh the device list */
  gm_pw_add_update_button (container, _("_Detect devices"), G_CALLBACK (refresh_devices_list_cb), _("Click here to refresh the device list"), 1, prefs_window);
}


static void
gm_pw_init_video_devices_page (GtkWidget *prefs_window,
                               GtkWidget *container)
{
  GmPreferencesWindow *pw = NULL;

  //GtkWidget *label = NULL;
  GtkWidget *subsection = NULL;

  //GtkWidget *button = NULL;

  //gchar *conf_image = NULL;
  //GtkFileFilter *filefilter = NULL;
  //GtkWidget *preview_image = NULL;
  //GtkWidget *preview_image_frame = NULL;

  PStringArray devs;

  gchar **array = NULL;

  gchar *video_size[NB_VIDEO_SIZES+1];
  unsigned int i;

  for (i=0; i< NB_VIDEO_SIZES; i++)
    video_size[i] = g_strdup_printf ("%dx%d", Ekiga::VideoSizes[i].width, Ekiga::VideoSizes[i].height);

  video_size [NB_VIDEO_SIZES] = NULL;

  const gchar *video_format [] =
    {
      _("PAL (Europe)"),
      _("NTSC (America)"),
      _("SECAM (France)"),
      _("Auto"),
      NULL
    };

  pw = gm_pw_get_pw (prefs_window);


  std::vector <std::string> device_list;


  /* The video devices related options */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
                                           _("Video Devices"), 5, 3);

  /* The video device */
  get_videoinput_devices (pw->videoinput_core, device_list);
  array = vector_of_string_to_array (device_list);
  pw->video_device =
    gnome_prefs_string_option_menu_new (subsection, _("Input device:"), (const gchar **)array, VIDEO_DEVICES_KEY "input_device", _("Select the video input device to use. If an error occurs when using this device a test picture will be transmitted."), 0, NULL);
  g_free (array);

  /* Video Channel */
  gnome_prefs_spin_new (subsection, _("Channel:"), VIDEO_DEVICES_KEY "channel", _("The video channel number to use (to select camera, tv or other sources)"), 0.0, 10.0, 1.0, 3, 3, NULL, false);

  gnome_prefs_int_option_menu_new (subsection, _("Size:"), (const gchar**)video_size, VIDEO_DEVICES_KEY "size", _("Select the transmitted video size"), 1);

  gnome_prefs_int_option_menu_new (subsection, _("Format:"), video_format, VIDEO_DEVICES_KEY "format", _("Select the format for video cameras (does not apply to most USB cameras)"), 2);

  /* That button will refresh the device list */
  gm_pw_add_update_button (container, _("_Detect devices"), G_CALLBACK (refresh_devices_list_cb), _("Click here to refresh the device list"), 1, prefs_window);

  for (i=0; i< NB_VIDEO_SIZES; i++)
    g_free (video_size[i]);
}


static void
gm_pw_init_audio_codecs_page (GtkWidget *prefs_window,
                              GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  GtkWidget *codecs_list = NULL;

  /* Packing widgets */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
				_("Codecs"), 1, 1);

  codecs_list = codecs_box_new_with_type (Ekiga::Call::Audio);
  gtk_grid_attach (GTK_GRID (subsection), codecs_list, 0, 0, 1, 1);

  /* Here we add the audio codecs options */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
				_("Settings"), 3, 1);

  /* Translators: the full sentence is Automatically adjust jitter buffer
     between X and Y ms */
  gnome_prefs_toggle_new (subsection, _("Enable silence _detection"), AUDIO_CODECS_KEY "enable_silence_detection", _("If enabled, use silence detection with the codecs supporting it"), 0, 1);

  gnome_prefs_toggle_new (subsection, _("Enable echo can_celation"), AUDIO_CODECS_KEY "enable_echo_cancellation", _("If enabled, use echo cancellation"), 1, 1);

  gnome_prefs_spin_new (subsection, _("Maximum _jitter buffer (in ms):"), AUDIO_CODECS_KEY "maximum_jitter_buffer", _("The maximum jitter buffer size for audio reception (in ms)"), 20.0, 2000.0, 50.0, 2, 1, NULL, true);
}


static void
gm_pw_init_video_codecs_page (GtkWidget *prefs_window,
                              GtkWidget *container)
{
  GtkWidget *subsection = NULL;
  GtkWidget *codecs_list = NULL;

  /* Packing widgets */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
				_("Codecs"), 1, 1);

  codecs_list = codecs_box_new_with_type (Ekiga::Call::Video);
  gtk_grid_attach (GTK_GRID (subsection), codecs_list, 0, 0, 1, 1);

  /* Here we add the video codecs options */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
				_("Settings"), 3, 1);

  /* Translators: the full sentence is Keep a minimum video quality of X % */
  gnome_prefs_scale_new (subsection, _("Picture quality"), _("Frame rate"), VIDEO_CODECS_KEY "temporal_spatial_tradeoff", _("Choose if you want to guarantee a minimum image quality (possibly leading to dropped frames in order not to surpass the bitrate limit) or if you prefer to keep the frame rate"), 0.0, 32.0, 1.0, 2);

  gnome_prefs_spin_new (subsection, _("Maximum video _bitrate (in kbits/s):"), VIDEO_CODECS_KEY "maximum_video_tx_bitrate", _("The maximum video bitrate in kbits/s. The video quality and the effective frame rate will be dynamically adjusted to keep the bitrate at the given value."), 16.0, 10240.0, 1.0, 1, 1, NULL, true);
}


/* GTK Callbacks */
static void
refresh_devices_list_cb (G_GNUC_UNUSED GtkWidget *widget,
			 gpointer data)
{
  g_return_if_fail (data != NULL);
  GtkWidget *prefs_window = GTK_WIDGET (data);

  gm_prefs_window_update_devices_list(prefs_window);
}


static void
audioev_filename_browse_cb (GtkWidget *b,
                            gpointer data)
{

  GmPreferencesWindow *pw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  gchar *filename = NULL;
  gchar *conf_key = NULL;
  gchar *sound_event = NULL;

  g_return_if_fail (data != NULL);
  pw = gm_pw_get_pw (GTK_WIDGET (data));

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
      2, &conf_key, -1);

    if (conf_key) {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (b));
      if (filename) {
        sound_event = gm_conf_get_string (conf_key);

        if (!sound_event || g_strcmp0 (filename, sound_event))
          gm_conf_set_string (conf_key, (gchar *) filename);

        g_free (filename);
      }

      g_free (conf_key);
      g_free (sound_event);
    }
  }
}


static void
sound_events_list_changed_nt (G_GNUC_UNUSED gpointer id,
			      GmConfEntry *entry,
			      gpointer data)
{
  GtkWidget *prefs_window = (GtkWidget*)data;

  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING
      || gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    if (prefs_window)
      gm_prefs_window_sound_events_list_build (prefs_window);
  }
}


static void
sound_event_clicked_cb (GtkTreeSelection *selection,
			gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *conf_key = NULL;
  gchar *filename = NULL;
  gchar *sound_event = NULL;

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			2, &conf_key, -1);

    if (conf_key) {

      sound_event = gm_conf_get_string (conf_key);

      if (sound_event) {

        if (!g_path_is_absolute (sound_event))
          filename = g_build_filename (DATA_DIR, "sounds", PACKAGE_NAME,
                                       sound_event, NULL);
        else
          filename = g_strdup (sound_event);

        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (data), filename);
        g_free (filename);
        g_free (sound_event);
      }

      g_free (conf_key);
    }
  }
}


static void
sound_event_play_cb (G_GNUC_UNUSED GtkWidget *widget,
		     gpointer data)
{
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter selected_iter;

  gchar *sound_event = NULL;

  GmPreferencesWindow *pw = NULL;
  
  g_return_if_fail (data != NULL);
  pw = gm_pw_get_pw (GTK_WIDGET (data));

  g_return_if_fail (data != NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  if (gtk_tree_selection_get_selected (selection, &model, &selected_iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &selected_iter, 4, &sound_event, -1);

    if (sound_event) {
      pw->audiooutput_core->play_event(sound_event);
      g_free (sound_event);
    }
  }
}


static void
sound_event_toggled_cb (G_GNUC_UNUSED GtkCellRendererToggle *cell,
			gchar *path_str,
			gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  gchar *conf_key = NULL;

  bool fixed = FALSE;


  model = (GtkTreeModel *) data;
  path = gtk_tree_path_new_from_string (path_str);

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &fixed, 3, &conf_key, -1);

  fixed ^= 1;

  gm_conf_set_bool (conf_key, fixed);

  g_free (conf_key);
  gtk_tree_path_free (path);
}


static void
audioev_filename_browse_play_cb (GtkWidget* /* playbutton */,
				 gpointer data)
{
  GmPreferencesWindow* pw = NULL;

  g_return_if_fail (data != NULL);

  pw = gm_pw_get_pw (GTK_WIDGET (data));

  gchar* file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (pw->fsbutton));
  std::string file_name_string = file_name;
  pw->audiooutput_core->play_file(file_name_string);

  g_free (file_name);
}

void on_videoinput_device_added_cb (const Ekiga::VideoInputDevice & device, bool isDesired, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gnome_prefs_string_option_menu_add (pw->video_device, (device.GetString()).c_str(),  isDesired ? TRUE : FALSE);
}

void on_videoinput_device_removed_cb (const Ekiga::VideoInputDevice & device, bool, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gnome_prefs_string_option_menu_remove(pw->video_device, (device.GetString()).c_str());
}

void on_audioinput_device_added_cb (const Ekiga::AudioInputDevice & device, bool isDesired, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gnome_prefs_string_option_menu_add (pw->audio_recorder, (device.GetString()).c_str(),  isDesired ? TRUE : FALSE);

}

void on_audioinput_device_removed_cb (const Ekiga::AudioInputDevice & device, bool, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gnome_prefs_string_option_menu_remove(pw->audio_recorder, (device.GetString()).c_str());
}

void on_audiooutput_device_added_cb (const Ekiga::AudioOutputDevice & device, bool isDesired,  GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gnome_prefs_string_option_menu_add (pw->audio_player, (device.GetString()).c_str(), isDesired ? TRUE : FALSE);
  gnome_prefs_string_option_menu_add (pw->sound_events_output, (device.GetString()).c_str(), isDesired ? TRUE : FALSE);
}

void on_audiooutput_device_removed_cb (const Ekiga::AudioOutputDevice & device, bool, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gnome_prefs_string_option_menu_remove(pw->audio_player, (device.GetString()).c_str());
  gnome_prefs_string_option_menu_remove(pw->sound_events_output, (device.GetString()).c_str());
}

/* Public functions */
void 
gm_prefs_window_update_devices_list (GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  gchar **array = NULL;

  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);

  std::vector <std::string> device_list;

  /* The player */
  get_audiooutput_devices (pw->audiooutput_core, device_list);
  array = vector_of_string_to_array (device_list);
  gnome_prefs_string_option_menu_update (pw->audio_player,
 					 (const gchar **)array,
 					 AUDIO_DEVICES_KEY "output_device",
 					 DEFAULT_AUDIO_DEVICE_NAME);
  gnome_prefs_string_option_menu_update (pw->sound_events_output,
                                         (const gchar **)array,
                                         SOUND_EVENTS_KEY "output_device",
                                         DEFAULT_AUDIO_DEVICE_NAME);
  g_free (array);

  /* The recorder */
  get_audioinput_devices (pw->audioinput_core, device_list);
  array = vector_of_string_to_array (device_list);
  gnome_prefs_string_option_menu_update (pw->audio_recorder,
 					 (const gchar **)array,
 					 AUDIO_DEVICES_KEY "input_device",
 					 DEFAULT_AUDIO_DEVICE_NAME);
  g_free (array);


  /* The Video player */
  get_videoinput_devices (pw->videoinput_core, device_list);
  array = vector_of_string_to_array (device_list);
  gnome_prefs_string_option_menu_update (pw->video_device,
					 (const gchar **)array,
					 VIDEO_DEVICES_KEY "input_device",
					 get_default_video_device_name (array));
  g_free (array);
}


GtkWidget *
preferences_window_new (Ekiga::ServiceCore& core)
{
  GmPreferencesWindow *pw = NULL;

  GdkPixbuf *pixbuf = NULL;
  GtkWidget *window = NULL;
  GtkWidget *container = NULL;
  gchar     *filename = NULL;
  std::vector <std::string> device_list;

  gpointer notifier;

  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME, PACKAGE_NAME "-logo.png", NULL);
  window = gnome_prefs_window_new (filename);
  g_free (filename);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("preferences_window"), g_free);
  gtk_window_set_title (GTK_WINDOW (window), _("Ekiga Preferences"));
  pixbuf = gtk_widget_render_icon_pixbuf (GTK_WIDGET (window),
					  GTK_STOCK_PREFERENCES,
					  GTK_ICON_SIZE_MENU);
  gtk_window_set_icon (GTK_WINDOW (window), pixbuf);
  gtk_widget_realize (GTK_WIDGET (window));
  g_object_unref (pixbuf);


  /* The GMObject data */
  pw = new GmPreferencesWindow;

  pw->audioinput_core = core.get<Ekiga::AudioInputCore> ("audioinput-core");
  pw->audiooutput_core = core.get<Ekiga::AudioOutputCore> ("audiooutput-core");
  pw->videoinput_core = core.get<Ekiga::VideoInputCore> ("videoinput-core");

  g_object_set_data_full (G_OBJECT (window), "GMObject",
			  pw, (GDestroyNotify) gm_pw_destroy);


  gnome_prefs_window_section_new (window, _("General"));
  container = gnome_prefs_window_subsection_new (window, _("Personal Data"));
  gm_pw_init_general_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("General Settings"));
  gm_pw_init_interface_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("Call Options"));
  gm_pw_init_call_options_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("Sound Events"));
  gm_pw_init_sound_events_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  gnome_prefs_window_section_new (window, _("Protocols"));
  container = gnome_prefs_window_subsection_new (window,
						 _("SIP Settings"));
  gm_pw_init_sip_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("H.323 Settings"));
  gm_pw_init_h323_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));


  /* The player */
  gnome_prefs_window_section_new (window, _("Audio"));
  container = gnome_prefs_window_subsection_new (window, _("Devices"));
  gm_pw_init_audio_devices_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("Codecs"));
  gm_pw_init_audio_codecs_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));


  gnome_prefs_window_section_new (window, _("Video"));
  container = gnome_prefs_window_subsection_new (window, _("Devices"));
  gm_pw_init_video_devices_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("Codecs"));
  gm_pw_init_video_codecs_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));


  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (window, "response",
			    G_CALLBACK (gm_window_hide),
			    (gpointer) window);

  gm_window_hide_on_delete (window);

  boost::signals::connection conn;

  conn = pw->videoinput_core->device_added.connect (boost::bind (&on_videoinput_device_added_cb, _1, _2, window));
  pw->connections.add (conn);
  conn = pw->videoinput_core->device_removed.connect (boost::bind (&on_videoinput_device_removed_cb, _1, _2, window));
  pw->connections.add (conn);

  conn = pw->audioinput_core->device_added.connect (boost::bind (&on_audioinput_device_added_cb, _1, _2, window));
  pw->connections.add (conn);
  conn = pw->audioinput_core->device_removed.connect (boost::bind (&on_audioinput_device_removed_cb, _1, _2, window));
  pw->connections.add (conn);

  conn = pw->audiooutput_core->device_added.connect (boost::bind (&on_audiooutput_device_added_cb, _1, _2, window));
  pw->connections.add(conn);
  conn = pw->audiooutput_core->device_removed.connect (boost::bind (&on_audiooutput_device_removed_cb, _1, _2, window));
  pw->connections.add (conn);

  /* Connect notifiers for SOUND_EVENTS_KEY keys */
  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_incoming_call_sound", 
			  sound_events_list_changed_nt, window);
   pw->notifiers.push_front (notifier);

  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "incoming_call_sound",
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);

  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_ring_tone_sound", 
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);
  
  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "ring_tone_sound", 
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);
  
  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_busy_tone_sound", 
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);
  
  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "busy_tone_sound",
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);
  
  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_new_voicemail_sound", 
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);
  
  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "new_voicemail_sound",
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);

  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_new_message_sound",
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);

  notifier =
    gm_conf_notifier_add (SOUND_EVENTS_KEY "new_message_sound",
			  sound_events_list_changed_nt, window);
  pw->notifiers.push_front (notifier);

  return window;
}
