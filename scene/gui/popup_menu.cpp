/*************************************************************************/
/*  popup_menu.cpp                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "popup_menu.h"

#include "core/input/input.h"
#include "core/os/keyboard.h"
#include "core/os/os.h"
#include "core/print_string.h"
#include "core/translation.h"

void PopupMenu::_gui_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventKey> k = p_event;

	if (allow_search && k.is_valid() && k->get_unicode() && k->is_pressed()) {
		uint64_t now = OS::get_singleton()->get_ticks_msec();
		uint64_t diff = now - search_time_msec;
		uint64_t max_interval = uint64_t(GLOBAL_DEF("gui/timers/incremental_search_max_interval_msec", 2000));
		search_time_msec = now;

		if (diff > max_interval) {
			search_string = "";
		}

		if (String::chr(k->get_unicode()) != search_string) {
			search_string += String::chr(k->get_unicode());
		}

		for (int i = 0; i < items.size(); i++) {
			Ref<Button> item(items.get(i));
			
			if (item.is_valid() && item->get_text().findn(search_string) == 0) {
				mouse_over = i;
				item->grab_focus();
				set_input_as_handled();
				break;
			}
		}
	}
}

void PopupMenu::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			PopupMenu *pm = Object::cast_to<PopupMenu>(get_parent());
			if (pm) {
				// Inherit submenu's popup delay time from parent menu
				float pm_delay = pm->get_submenu_popup_delay();
				set_submenu_popup_delay(pm_delay);
			}
		} break;
		case NOTIFICATION_WM_MOUSE_ENTER: {
			//grab_focus();
		} break;
		case NOTIFICATION_WM_MOUSE_EXIT: {
		} break;
		case NOTIFICATION_POST_POPUP: {
			initial_button_mask = Input::get_singleton()->get_mouse_button_mask();
			during_grabbed_click = (bool)initial_button_mask;
		} break;
		case NOTIFICATION_WM_SIZE_CHANGED: {
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			//only used when using operating system windows
			if (get_window_id() != DisplayServer::INVALID_WINDOW_ID && autohide_areas.size()) {
				Point2 mouse_pos = DisplayServer::get_singleton()->mouse_get_position();
				mouse_pos -= get_position();

				for (List<Rect2>::Element *E = autohide_areas.front(); E; E = E->next()) {
					if (!Rect2(Point2(), get_size()).has_point(mouse_pos) && E->get().has_point(mouse_pos)) {
						_close_pressed();
						return;
					}
				}
			}
		} break;
	}
}

/* Methods to add items with or without icon, checkbox, shortcut.
 * Be sure to keep them in sync when adding new properties in the Item struct.
 */

#define ITEM_SETUP_WITH_ACCEL(p_label, p_id, p_accel) \
	item.text = p_label;                              \
	item.xl_text = tr(p_label);                       \
	item.id = p_id == -1 ? items.size() : p_id;       \
	item.accel = p_accel;

/* Methods to modify existing items. */

int PopupMenu::get_current_index() const {
	return mouse_over;
}

int PopupMenu::get_item_count() const {
	return items.size();
}

/*bool PopupMenu::activate_item_by_event(const Ref<InputEvent> &p_event, bool p_for_global_only) {
	uint32_t code = 0;
	Ref<InputEventKey> k = p_event;

	if (k.is_valid()) {
		code = k->get_keycode();
		if (code == 0) {
			code = k->get_unicode();
		}
		if (k->get_control()) {
			code |= KEY_MASK_CTRL;
		}
		if (k->get_alt()) {
			code |= KEY_MASK_ALT;
		}
		if (k->get_metakey()) {
			code |= KEY_MASK_META;
		}
		if (k->get_shift()) {
			code |= KEY_MASK_SHIFT;
		}
	}

	for (int i = 0; i < items.size(); i++) {
		Ref<Button> item(items.get(i));
		
		if (item->is_disabled()) {
			continue;
		}

		if (item->get_shortcut().is_valid() && item->get_shortcut()->is_shortcut(p_event)) {
			//activate_item(i);
			item->set_pressed(true);
			return true;
		}

		/*if (code != 0 && items[i].accel == code) {
			activate_item(i);
			return true;
		}*/

		/*if (items[i].submenu != "") {
			Node *n = get_node(items[i].submenu);
			if (!n) {
				continue;
			}

			PopupMenu *pm = Object::cast_to<PopupMenu>(n);
			if (!pm) {
				continue;
			}

			if (pm->activate_item_by_event(p_event, p_for_global_only)) {
				return true;
			}
		}* /
	}
	return false;
}*/

/*void PopupMenu::activate_item(int p_item) {
	get_item(p_item)->grab_focus();
}*/

void PopupMenu::_select_item(Control *p_item) {
	int index = _get_items().find(p_item);
	emit_signal("selected", index);
	hide();
}

void PopupMenu::add_item(Control *p_item) {
	items.push_back(p_item);
	item_container->add_child(p_item);
}

Button *PopupMenu::add_button(const String &p_label, Ref<ShortCut> p_shortcut, const Ref<Texture2D> p_icon) {
	Button *button = memnew(Button);
	button->set_text(p_label);
	button->set_shortcut(p_shortcut);
	button->set_icon(p_icon);
	button->connect("pressed", callable_mp(this, &PopupMenu::_select_item), varray(button));
	add_item(button);
	return button;
}

CheckBox *PopupMenu::add_check_button(const String &p_label, Ref<ShortCut> p_shortcut, const Ref<Texture2D> p_icon) {
	CheckBox *button = memnew(CheckBox);
	button->set_text(p_label);
	button->set_shortcut(p_shortcut);
	button->set_icon(p_icon);
	add_item(button);
	return button;
}

CheckBox *PopupMenu::add_radio_button(const String &p_label, Ref<ButtonGroup> p_group, Ref<ShortCut> p_shortcut, Ref<Texture2D> p_icon) {
	CheckBox *button = memnew(CheckBox);
	button->set_text(p_label);
	button->set_button_group(p_group);
	button->set_shortcut(p_shortcut);
	button->set_icon(p_icon);
	add_item(button);
	return button;
}

Button *PopupMenu::add_icon_button(const String &p_label, Ref<Texture2D> p_icon, Ref<ShortCut> p_shortcut) {
	return add_button(p_label, p_shortcut, p_icon);
}

Label *PopupMenu::add_label(const String &p_label) {
	Label *label = memnew(Label);
	label->set_text(p_label);
	add_item(label);
	return label;
}

SplitContainer *PopupMenu::add_separator() {
	SplitContainer *sep = memnew(VSplitContainer);
	add_item(sep);
	return sep;
}

Control *PopupMenu::get_item(const int &p_id) {
	return items.get(p_id);
}

Button *PopupMenu::get_button(const int &p_id) {
	Button *button = Object::cast_to<Button>(get_item(p_id));
	ERR_FAIL_COND_V_MSG(!button, nullptr, "Item at " + Variant(p_id) + " is not a button");
	return button;
}

CheckBox *PopupMenu::get_check_button(const int &p_id) {
	CheckBox *button = Object::cast_to<CheckBox>(get_item(p_id));
	ERR_FAIL_COND_V_MSG(!button, nullptr, "Item at " + Variant(p_id) + " is not a check button");
	return button;
}

CheckBox *PopupMenu::get_radio_button(const int &p_id) {
	CheckBox *button = Object::cast_to<CheckBox>(get_item(p_id));
	ERR_FAIL_COND_V_MSG(!button, nullptr, "Item at " + Variant(p_id) + " is not a radio button");
	return button;
}

void PopupMenu::remove_item(const int &p_id) {
	items.remove(p_id);
	item_container->remove_child(item_container->get_child(p_id));
}

void PopupMenu::clear() {
	for (int i = 0; i < items.size(); i++) {
		remove_item(0);
	}
}

void PopupMenu::_ref_shortcut(Ref<ShortCut> p_sc) {
	if (!shortcut_refcount.has(p_sc)) {
		shortcut_refcount[p_sc] = 1;
		p_sc->connect("changed", callable_mp((CanvasItem *)this, &CanvasItem::update));
	} else {
		shortcut_refcount[p_sc] += 1;
	}
}

void PopupMenu::_unref_shortcut(Ref<Shortcut> p_sc) {
	ERR_FAIL_COND(!shortcut_refcount.has(p_sc));
	shortcut_refcount[p_sc]--;
	if (shortcut_refcount[p_sc] == 0) {
		p_sc->disconnect("changed", callable_mp((CanvasItem *)this, &CanvasItem::update));
		shortcut_refcount.erase(p_sc);
	}
}

// Hide on item selection determines whether or not the popup will close after item selection
void PopupMenu::set_hide_on_item_selection(bool p_enabled) {
	hide_on_item_selection = p_enabled;
}

bool PopupMenu::is_hide_on_item_selection() const {
	return hide_on_item_selection;
}

void PopupMenu::set_hide_on_checkable_item_selection(bool p_enabled) {
	hide_on_checkable_item_selection = p_enabled;
}

bool PopupMenu::is_hide_on_checkable_item_selection() const {
	return hide_on_checkable_item_selection;
}

void PopupMenu::set_hide_on_multistate_item_selection(bool p_enabled) {
	hide_on_multistate_item_selection = p_enabled;
}

bool PopupMenu::is_hide_on_multistate_item_selection() const {
	return hide_on_multistate_item_selection;
}

void PopupMenu::set_submenu_popup_delay(float p_time) {
	if (p_time <= 0) {
		p_time = 0.01;
	}

	submenu_timer->set_wait_time(p_time);
}

float PopupMenu::get_submenu_popup_delay() const {
	return submenu_timer->get_wait_time();
}

void PopupMenu::set_allow_search(bool p_allow) {
	allow_search = p_allow;
}

bool PopupMenu::get_allow_search() const {
	return allow_search;
}

/*String PopupMenu::get_tooltip(const Point2 &p_pos) const {
	int over = _get_mouse_over(p_pos);
	if (over < 0 || over >= items.size()) {
		return "";
	}
	return nget_item(over).tooltip;
}*/

void PopupMenu::set_parent_rect(const Rect2 &p_rect) {
	parent_rect = p_rect;
}

/*void PopupMenu::get_translatable_strings(List<String> *p_strings) const {
	for (int i = 0; i < items.size(); i++) {
		if (items[i].xl_text != "") {
			p_strings->push_back(items[i].xl_text);
		}
	}
}*/

void PopupMenu::add_autohide_area(const Rect2 &p_area) {
	autohide_areas.push_back(p_area);
}

void PopupMenu::clear_autohide_areas() {
	autohide_areas.clear();
}

void PopupMenu::take_mouse_focus() {
	ERR_FAIL_COND(!is_inside_tree());

	if (get_parent()) {
		get_parent()->get_viewport()->pass_mouse_focus_to(this, item_container);
	}
}

void PopupMenu::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_gui_input"), &PopupMenu::_gui_input);

	ClassDB::bind_method(D_METHOD("add_item", "item"), &PopupMenu::add_item);
	ClassDB::bind_method(D_METHOD("add_button", "label"), &PopupMenu::add_button);
	ClassDB::bind_method(D_METHOD("add_icon_button", "label"), &PopupMenu::add_button);

	ClassDB::bind_method(D_METHOD("get_current_index"), &PopupMenu::get_current_index);
	ClassDB::bind_method(D_METHOD("get_item_count"), &PopupMenu::get_item_count);

	ClassDB::bind_method(D_METHOD("remove_item", "idx"), &PopupMenu::remove_item);

	ClassDB::bind_method(D_METHOD("add_separator"), &PopupMenu::add_separator);
	ClassDB::bind_method(D_METHOD("clear"), &PopupMenu::clear);

	ClassDB::bind_method(D_METHOD("_set_items"), &PopupMenu::_set_items);
	ClassDB::bind_method(D_METHOD("_get_items"), &PopupMenu::_get_items);

	ClassDB::bind_method(D_METHOD("set_hide_on_item_selection", "enable"), &PopupMenu::set_hide_on_item_selection);
	ClassDB::bind_method(D_METHOD("is_hide_on_item_selection"), &PopupMenu::is_hide_on_item_selection);

	ClassDB::bind_method(D_METHOD("set_hide_on_checkable_item_selection", "enable"), &PopupMenu::set_hide_on_checkable_item_selection);
	ClassDB::bind_method(D_METHOD("is_hide_on_checkable_item_selection"), &PopupMenu::is_hide_on_checkable_item_selection);

	ClassDB::bind_method(D_METHOD("set_hide_on_state_item_selection", "enable"), &PopupMenu::set_hide_on_multistate_item_selection);
	ClassDB::bind_method(D_METHOD("is_hide_on_state_item_selection"), &PopupMenu::is_hide_on_multistate_item_selection);

	ClassDB::bind_method(D_METHOD("set_submenu_popup_delay", "seconds"), &PopupMenu::set_submenu_popup_delay);
	ClassDB::bind_method(D_METHOD("get_submenu_popup_delay"), &PopupMenu::get_submenu_popup_delay);

	ClassDB::bind_method(D_METHOD("set_allow_search", "allow"), &PopupMenu::set_allow_search);
	ClassDB::bind_method(D_METHOD("get_allow_search"), &PopupMenu::get_allow_search);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "items", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_items", "_get_items");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hide_on_item_selection"), "set_hide_on_item_selection", "is_hide_on_item_selection");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hide_on_checkable_item_selection"), "set_hide_on_checkable_item_selection", "is_hide_on_checkable_item_selection");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hide_on_state_item_selection"), "set_hide_on_state_item_selection", "is_hide_on_state_item_selection");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "submenu_popup_delay"), "set_submenu_popup_delay", "get_submenu_popup_delay");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_search"), "set_allow_search", "get_allow_search");

	ADD_SIGNAL(MethodInfo("selected", PropertyInfo(Variant::INT, "index")));
}

void PopupMenu::popup(const Rect2 &p_bounds) {
	moved = Vector2();
	popup_time_msec = OS::get_singleton()->get_ticks_msec();
	set_as_minsize();
	Popup::popup(p_bounds);
}

PopupMenu::PopupMenu() {
	// Margin Container
	margin_container = memnew(MarginContainer);
	margin_container->set_anchors_and_margins_preset(Control::PRESET_WIDE);
	add_child(margin_container);

	// Scroll Container
	scroll_container = memnew(ScrollContainer);
	scroll_container->set_clip_contents(true);
	scroll_container->set_follow_focus(true);
	margin_container->add_child(scroll_container);

	// The control which will display the items
	item_container = memnew(VBoxContainer);
	item_container->set_clip_contents(false);
	item_container->set_anchors_and_margins_preset(Control::PRESET_WIDE);
	item_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	item_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	scroll_container->add_child(item_container);

	connect("window_input", callable_mp(this, &PopupMenu::_gui_input));

	mouse_over = -1;
	submenu_over = -1;
	initial_button_mask = 0;
	during_grabbed_click = false;

	allow_search = true;
	search_time_msec = 0;
	search_string = "";

	set_hide_on_item_selection(true);
	set_hide_on_checkable_item_selection(true);
	set_hide_on_multistate_item_selection(false);

	submenu_timer = memnew(Timer);
	submenu_timer->set_wait_time(0.3);
	submenu_timer->set_one_shot(true);
	submenu_timer->connect("timeout", callable_mp(this, &PopupMenu::_submenu_timeout));
	add_child(submenu_timer);
}

PopupMenu::~PopupMenu() {
}
