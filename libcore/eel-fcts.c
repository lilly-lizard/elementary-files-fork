/* 
 * Copyright (C) 1999, 2000, 2001 Eazel, Inc.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: John Sullivan <sullivan@eazel.com>
 *          Darin Adler <darin@bentspoon.com>
 */


#include "eel-fcts.h"

#include <glib-object.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "eel-glib-extensions.h"
#include "eel-i18n.h"

static const char *TODAY_TIME_FORMATS [] = {
    /* Today, use special word.
     * strftime patterns preceeded with the widest
     * possible resulting string for that pattern.
     *
     * Note to localizers: You can look at man strftime
     * for details on the format, but you should only use
     * the specifiers from the C standard, not extensions.
     * These include "%" followed by one of
     * "aAbBcdHIjmMpSUwWxXyYZ". There are two extensions
     * in the Nautilus version of strftime that can be
     * used (and match GNU extensions). Putting a "-"
     * between the "%" and any numeric directive will turn
     * off zero padding, and putting a "_" there will use
     * space padding instead of zero padding.
     */
    N_("Today at 00:00:00 PM"),
    N_("Today at %-I:%M:%S %p"),

    N_("Today at 00:00:00 PM"),
    N_("Today at %-I:%M:%S %p"),

    N_("Today at 00:00 PM"),
    N_("Today at %-I:%M %p"),

    N_("Today, 00:00 PM"),
    N_("Today, %-I:%M %p"),

    N_("Today"),
    N_("Today"),

    NULL
};

static const char *YESTERDAY_TIME_FORMATS [] = {
    /* Yesterday, use special word.
     * Note to localizers: Same issues as "Today" string.
     */
    N_("Yesterday at 00:00:00 PM"),
    N_("Yesterday at %-I:%M:%S %p"),

    N_("Yesterday at 00:00:00 PM"),
    N_("Yesterday at %-I:%M:%S %p"),

    N_("Yesterday at 00:00 PM"),
    N_("Yesterday at %-I:%M %p"),

    N_("Yesterday, 00:00 PM"),
    N_("Yesterday, %-I:%M %p"),

    N_("Yesterday"),
    N_("Yesterday"),

    NULL
};

static const char *CURRENT_WEEK_TIME_FORMATS [] = {
    /* Current week, include day of week.
     * Note to localizers: Same issues as "Today" string.
     * The width measurement templates correspond to
     * the day/month name with the most letters.
     */
    N_("Wednesday, September 00 0000 at 00:00:00 PM"),
    N_("%A, %B %-d %Y at %-I:%M:%S %p"),

    N_("Mon, Oct 00 0000 at 00:00:00 PM"),
    N_("%a, %b %-d %Y at %-I:%M:%S %p"),

    /*N_("Mon, Oct 00 0000 at 00:00 PM"),
    N_("%a, %b %-d %Y at %-I:%M %p"),*/
    N_("Mon 00 Oct 0000 at 00:00 PM"),
    N_("%a %-d %b %Y at %-I:%M %p"),

    N_("Oct 00 0000 at 00:00 PM"),
    N_("%b %-d %Y at %-I:%M %p"),

    N_("Oct 00 0000, 00:00 PM"),
    N_("%b %-d %Y, %-I:%M %p"),

    N_("00/00/00, 00:00 PM"),
    N_("%m/%-d/%y, %-I:%M %p"),

    N_("00/00/00"),
    N_("%m/%d/%y"),

    NULL
};

/**
 * eel_get_date_as_string:
 * 
 * Get a formated date string where format equal iso, locale, informal.
 * The caller is responsible for g_free-ing the result.
 * @d: contains the UNIX time.
 * @date_format: string representing the format to convert the date to.
 * 
 * Returns: Newly allocated date.
 * 
**/
char *
eel_get_date_as_string (guint64 d, gchar *date_format)
{
    const char **formats;
    //const char *width_template;
    const char *format;
    gchar *result = NULL;
    //int i;
    GDateTime *date_time, *today;
    GTimeSpan file_date_age;

    g_return_val_if_fail (date_format != NULL, NULL);
    date_time = g_date_time_new_from_unix_local ((gint64) d);

    if (!strcmp (date_format, "locale")) {
        result = g_date_time_format (date_time, "%c");
		goto out;
    } else if (!strcmp (date_format, "iso")) {
        result = g_date_time_format (date_time, "%Y-%m-%d %H:%M:%S");
		goto out;
    }

    today = g_date_time_new_now_local ();
	file_date_age = g_date_time_difference (today, date_time);
	g_date_time_unref (today);

    /* Format varies depending on how old the date is. This minimizes
     * the length (and thus clutter & complication) of typical dates
     * while providing sufficient detail for recent dates to make
     * them maximally understandable at a glance. Keep all format
     * strings separate rather than combining bits & pieces for
     * internationalization's sake.
     */

    if (file_date_age < G_TIME_SPAN_DAY) {
        formats = TODAY_TIME_FORMATS;
    } else if (file_date_age < 2 * G_TIME_SPAN_DAY) {
        formats = YESTERDAY_TIME_FORMATS;
    } else {
        formats = CURRENT_WEEK_TIME_FORMATS;
    }

#if 0
    /* Find the date format that just fits the required width. Instead of measuring
     * the resulting string width directly, measure the width of a template that represents
     * the widest possible version of a date in a given format. This is done by using M, m
     * and 0 for the variable letters/digits respectively.
     */
    format = NULL;

    for (i = 0; ; i += 2) {
        width_template = (formats [i] ? _(formats [i]) : NULL);
        if (width_template == NULL) {
            /* no more formats left */
            g_assert (format != NULL);

            /* Can't fit even the shortest format -- return an ellipsized form in the
             * shortest format
             */

            result = g_date_time_format (date_time, format);
        }

        format = _(formats [i + 1]);

        /* don't care about fitting the width */
        break;
    }

    if (result == NULL) {
		result = g_date_time_format (date_time, format);
	}
#endif
    format = _(formats [5]);
	result = g_date_time_format (date_time, format);

 out:
    g_date_time_unref (date_time);
    return result;
}

/**
 * eel_get_user_names:
 * 
 * Get a list of user names. 
 * The caller is responsible for freeing this list and its contents.
 */
GList *
eel_get_user_names (void)
{
	GList *list;
	struct passwd *user;

	list = NULL;
	setpwent ();
	while ((user = getpwent ()) != NULL) {
		list = g_list_prepend (list, g_strdup (user->pw_name));
	}
	endpwent ();

	return eel_g_str_list_alphabetize (list);
}

/* Get a list of group names, filtered to only the ones
 * that contain the given username. If the username is
 * NULL, returns a list of all group names.
 */
GList *
eel_get_group_names_for_user (void)
{
	GList *list;
	struct group *group;
	int count, i;
	gid_t gid_list[NGROUPS_MAX + 1];
	

	list = NULL;

	count = getgroups (NGROUPS_MAX + 1, gid_list);
	for (i = 0; i < count; i++) {
		group = getgrgid (gid_list[i]);
		if (group == NULL)
			break;
		
		list = g_list_prepend (list, g_strdup (group->gr_name));
	}

	return eel_g_str_list_alphabetize (list);
}

/**
 * Get a list of all group names.
 */
GList *
eel_get_all_group_names (void)
{
	GList *list;
	struct group *group;
	
	list = NULL;

	setgrent ();
	
	while ((group = getgrent ()) != NULL)
		list = g_list_prepend (list, g_strdup (group->gr_name));
	
	endgrent ();
	
	return eel_g_str_list_alphabetize (list);
}

gboolean
eel_get_group_id_from_group_name (const char *group_name, uid_t *gid)
{
	struct group *group;

	g_assert (gid != NULL);

	group = getgrnam (group_name);

	if (group == NULL)
		return FALSE;

	*gid = group->gr_gid;

	return TRUE;
}

gboolean
eel_get_user_id_from_user_name (const char *user_name, uid_t *uid)
{
	struct passwd *password_info;

	g_assert (uid != NULL);

	password_info = getpwnam (user_name);

	if (password_info == NULL)
        return FALSE;

	*uid = password_info->pw_uid;

	return TRUE;
}

gboolean
eel_get_id_from_digit_string (const char *digit_string, uid_t *id)
{
	long scanned_id;
	char c;

	g_assert (id != NULL);

	/* Only accept string if it has one integer with nothing
	 * afterwards.
	 */
	if (sscanf (digit_string, "%ld%c", &scanned_id, &c) != 1) {
		return FALSE;
	}
	*id = scanned_id;
	return TRUE;
}

/* TODO check again sanity of g_format_size. 
standby too much segfault from this function */

/* TODO remove this once format_size has populated glib-2.0.vapi 
 * g_format_size_for_display is deprectaed since glib 2.30 
 */
gchar 
*eel_format_size (guint64 size)
{
    return g_format_size (size);
}
