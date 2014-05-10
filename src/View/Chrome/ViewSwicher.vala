//
//  ViewSwicher.cs
//
//  Authors:
//       mathijshenquet <mathijs.henquet@gmail.com>
//       ammonkey <am.monkeyd@gmail.com>
//
//  Copyright (c) 2010 mathijshenquet
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

namespace Marlin.View.Chrome {

    public class ViewSwitcher : Gtk.Box {
        public Granite.Widgets.ModeButton switcher;

        private ViewMode _mode;
        public ViewMode mode {
        //private ViewMode mode{
            set {
                Gtk.Widget target;
                int active_index;

                switch (value) {
                case ViewMode.LIST:
                    target = list;
                    active_index = 1;
                    break;
                case ViewMode.MILLER:
                    target = miller;
                    active_index = 2;
                    break;
                default:
                    target = icon;
                    active_index = 0;
                    break;
                }

                Preferences.settings.set_enum ("default-viewmode", value);
                //switcher.focus(target);
                switcher.set_active (active_index);
                _mode = mode;
            }
            private get {
                return _mode;
            }
        }

        private Gtk.ActionGroup main_actions;

        private Gtk.Image icon;
        private Gtk.Image list;
        private Gtk.Image miller;

        public ViewSwitcher (Gtk.ActionGroup action_group) {
            Object (orientation: Gtk.Orientation.HORIZONTAL);

            main_actions = action_group;

            switcher = new Granite.Widgets.ModeButton ();
            switcher.halign = switcher.valign = Gtk.Align.CENTER;

            icon = new Gtk.Image.from_icon_name ("view-grid-symbolic", Gtk.IconSize.MENU);
            icon.tooltip_text = _("View as Grid");
            switcher.append (icon);
            list = new Gtk.Image.from_icon_name ("view-list-symbolic", Gtk.IconSize.MENU);
            list.tooltip_text  = _("View as List");
            switcher.append (list);
            miller = new Gtk.Image.from_icon_name ("view-column-symbolic", Gtk.IconSize.MENU);
            miller.tooltip_text = _("View in Columns");
            switcher.append (miller);

            mode = (ViewMode) Preferences.settings.get_enum("default-viewmode");

            switcher.mode_changed.connect ((mode) => {
                Gtk.Action action;

                //You cannot do a switch here, only for int and string
                if (mode == list) {
                    action = main_actions.get_action ("view-as-detailed-list");
                    action.activate ();
                } else if (mode == miller) {
                    action = main_actions.get_action ("view-as-columns");
                    action.activate ();
                } else {
                    action = main_actions.get_action ("view-as-icons");
                    action.activate ();
                }
            });

            switcher.sensitive = true;

            pack_start (switcher, true, true, 0);
        }
    }
}